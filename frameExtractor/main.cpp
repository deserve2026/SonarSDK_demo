/*!
 * FrameExtractor — Standalone offline frame extraction tool
 *
 * Reads Lanheng forward-looking sonar .DB files and extracts individual
 * frames as PNG images for YOLO annotation / training.
 *
 * Interval selection guidance (1.2MHz sonar, 2m detection range):
 *   - Sonar frame rate is approximately 8–15 fps (depends on workCycle)
 *   - Fish swim at roughly 0.3–0.8 m/s in aquaculture settings
 *   - A fish crosses the 2m FOV in 2.5–7 seconds
 *
 *   Time mode (-m time):
 *     1s  → dense sampling, fish moves ~0.3–0.8m between frames
 *     2s  → balanced (recommended default), ~0.6–1.6m displacement
 *     3s+ → sparse sampling, fewer images to annotate
 *
 *   Frame-count mode (-m frame):
 *     Every 10 frames @ 15fps ≈ 0.67s between saved images
 *     Every 20 frames @ 15fps ≈ 1.33s
 *     Every 30 frames @ 15fps ≈ 2.0s  (recommended starting point)
 *
 * Usage:
 *   FrameExtractor <input.db> <output_dir> [-i N] [-m time|frame]
 *
 * Examples:
 *   FrameExtractor data.db ./frames
 *   FrameExtractor data.db ./frames -i 2 -m time
 *   FrameExtractor data.db ./frames -i 15 -m frame
 */

#include <QGuiApplication>
#include <QImage>
#include <QString>
#include <QDir>
#include <QTextStream>

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>

#include "LhForwardFrame.h"
#include "LhForwardImage.h"
#include "sonarCommon.h"

using namespace LhForwardSDK;

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------

struct FrameMeta {
    uint64_t byteOffset;       // byte offset in .DB file
    uint64_t timestampUs;      // frame timestamp in microseconds
    uint32_t frameSerialNum;
    uint8_t  packType;
};

struct ExtractConfig {
    QString   inputFile;
    QString   outputDir;
    double    interval   = 2.0;   // seconds (time) or frame step (frame-count)
    enum Mode { TIME, FRAME_COUNT } mode = TIME;
};

// ---------------------------------------------------------------------------
// CLI parser
// ---------------------------------------------------------------------------

static void printHelp(QTextStream &out)
{
    out << "\n"
        << "FrameExtractor — Offline sonar frame extraction for YOLO training\n"
        << "================================================================\n"
        << "\n"
        << "Usage:\n"
        << "  FrameExtractor <input.db> <output_dir> [-i N] [-m time|frame]\n"
        << "\n"
        << "Arguments:\n"
        << "  <input.db>       Path to the .DB sonar data file\n"
        << "  <output_dir>     Directory to save extracted PNG frames\n"
        << "\n"
        << "Options:\n"
        << "  -i, --interval N  Extraction interval (default: 2)\n"
        << "                      time mode  → N seconds between saved frames\n"
        << "                      frame mode → save every Nth frame\n"
        << "  -m, --mode MODE   'time' or 'frame' (default: time)\n"
        << "  -h, --help        Show this help\n"
        << "\n"
        << "Interval selection guidance (1.2 MHz sonar, 2 m range):\n"
        << "  Sonar frame rate is roughly 8–15 fps; a fish (~0.5 m/s)\n"
        << "  crosses the 2 m field-of-view in about 4 seconds.\n"
        << "\n"
        << "  Time mode:      1s = dense   2s = balanced   3s+ = sparse\n"
        << "  Frame mode:    ~10 = dense  ~20 = balanced  ~30 = sparse\n"
        << "\n"
        << "Examples:\n"
        << "  FrameExtractor data.db ./frames\n"
        << "  FrameExtractor data.db ./frames -i 2 -m time\n"
        << "  FrameExtractor data.db ./frames -i 15 -m frame\n"
        << "\n";
}

static bool parseArgs(int argc, char *argv[], ExtractConfig &config, QTextStream &err)
{
    if (argc < 3) {
        printHelp(err);
        return false;
    }

    config.inputFile  = QString::fromLocal8Bit(argv[1]);
    config.outputDir  = QString::fromLocal8Bit(argv[2]);

    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];

        if ((arg == "-i" || arg == "--interval") && i + 1 < argc) {
            config.interval = std::stod(argv[++i]);
            if (config.interval <= 0) {
                err << "Error: interval must be > 0\n";
                return false;
            }
        } else if ((arg == "-m" || arg == "--mode") && i + 1 < argc) {
            std::string mode = argv[++i];
            if (mode == "frame" || mode == "f")
                config.mode = ExtractConfig::FRAME_COUNT;
            else if (mode == "time" || mode == "t")
                config.mode = ExtractConfig::TIME;
            else {
                err << "Error: unknown mode '" << mode.c_str()
                    << "'. Use 'time' or 'frame'.\n";
                return false;
            }
        } else if (arg == "-h" || arg == "--help") {
            printHelp(err);
            return false;
        } else {
            err << "Error: unknown option '" << arg.c_str() << "'\n";
            return false;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Pre-scan: locate every frame in the .DB file
// ---------------------------------------------------------------------------

static std::vector<FrameMeta> prescanFile(const std::string &filePath, uint16_t packSize)
{
    std::vector<FrameMeta> frames;
    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "Error: cannot open file: " << filePath << "\n";
        return frames;
    }

    ifs.seekg(0, std::ios::end);
    uint64_t fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    if (fileSize < packSize) {
        std::cerr << "Error: file too small ("
                  << fileSize << " bytes < " << packSize << " byte packet)\n";
        return frames;
    }

    // Only read 256-byte headers during pre-scan (22× faster than full packets)
    const uint16_t HEADER_SIZE = PACK_INFO_SIZE;
    char header[HEADER_SIZE];
    packageInfo curHead;
    memset(&curHead, 0, sizeof(packageInfo));

    uint64_t totalPackets = fileSize / packSize;

    std::cout << "Pre-scanning " << (fileSize / (1024.0 * 1024.0))
              << " MB (" << totalPackets << " packets)...\n";

    for (uint64_t p = 0; p < totalPackets; ++p) {
        ifs.read(header, HEADER_SIZE);
        const packageInfo *head = reinterpret_cast<const packageInfo *>(header);

        if (head->frameSerialNum != curHead.frameSerialNum) {
            FrameMeta meta;
            meta.byteOffset      = p * packSize;
            meta.timestampUs     = (static_cast<uint64_t>(head->packTime1) << 16)
                                   | head->packTime2;
            meta.frameSerialNum  = head->frameSerialNum;
            meta.packType        = head->packType;
            frames.push_back(meta);
            memcpy(&curHead, head, sizeof(packageInfo));
        }

        // Seek past payload to next packet header
        ifs.seekg(packSize - HEADER_SIZE, std::ios::cur);

        // Progress dot every 10%
        if (totalPackets >= 10 && p % (totalPackets / 10) == 0 && p > 0)
            std::cout << "  " << (p * 100 / totalPackets) << "%" << std::endl;
    }

    std::cout << "Done. Found " << frames.size() << " frames.\n";
    return frames;
}

// ---------------------------------------------------------------------------
// Frame selection (time-based or frame-count-based)
// ---------------------------------------------------------------------------

static std::vector<size_t> selectFrames(const std::vector<FrameMeta> &allFrames,
                                        const ExtractConfig &config)
{
    std::vector<size_t> selected;
    if (allFrames.empty())
        return selected;

    if (config.mode == ExtractConfig::FRAME_COUNT) {
        // Every Nth frame (1-based step from the user perspective)
        int step = std::max(1, static_cast<int>(config.interval));
        for (size_t i = 0; i < allFrames.size(); i += step)
            selected.push_back(i);
    } else {
        // Time-based: save frames spaced by at least `interval` seconds
        selected.push_back(0);
        uint64_t lastTs = allFrames[0].timestampUs;
        double intervalUs = config.interval * 1'000'000.0;

        for (size_t i = 1; i < allFrames.size(); ++i) {
            int64_t delta = static_cast<int64_t>(allFrames[i].timestampUs) -
                            static_cast<int64_t>(lastTs);
            if (delta < 0) {
                // Timestamp wrapped — reset reference
                lastTs = allFrames[i].timestampUs;
                continue;
            }
            if (static_cast<double>(delta) >= intervalUs) {
                selected.push_back(i);
                lastTs = allFrames[i].timestampUs;
            }
        }
    }

    return selected;
}

// ---------------------------------------------------------------------------
// Main extraction logic
// ---------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    // QGuiApplication is required for QImage::save (PNG plugin)
    QGuiApplication app(argc, argv);

    QTextStream err(stderr);

    // --- parse arguments ---------------------------------------------------
    ExtractConfig config;
    if (!parseArgs(argc, argv, config, err))
        return 1;

    // --- validate input file -----------------------------------------------
    std::ifstream ifs(config.inputFile.toStdString(), std::ios::binary);
    if (!ifs.is_open()) {
        err << "Error: cannot open input file: " << config.inputFile << "\n";
        return 1;
    }
    ifs.seekg(0, std::ios::end);
    uint64_t fileSize = ifs.tellg();
    ifs.close();

    const uint16_t PACK_SIZE = LhForwardFrame::PACK_SIZE;
    if (fileSize < PACK_SIZE) {
        err << "Error: file too small ("
            << fileSize << " bytes < " << PACK_SIZE << " byte packet)\n";
        return 1;
    }

    // --- create output directory -------------------------------------------
    QDir dir;
    if (!dir.mkpath(config.outputDir)) {
        err << "Error: cannot create output directory: " << config.outputDir << "\n";
        return 1;
    }

    // --- pre-scan ----------------------------------------------------------
    auto allFrames = prescanFile(config.inputFile.toStdString(), PACK_SIZE);
    if (allFrames.empty()) {
        err << "Error: no frames found in file.\n";
        return 1;
    }

    // --- select frames to extract ------------------------------------------
    auto selected = selectFrames(allFrames, config);

    const char *modeLabel = (config.mode == ExtractConfig::TIME) ? "time" : "frame-count";
    std::cout << "\nExtracting " << selected.size() << " / " << allFrames.size()
              << " frames (" << modeLabel << " mode, interval = "
              << config.interval << ")\n\n";

    // --- setup SDK objects -------------------------------------------------
    LhForwardFrame  frameAssembler;
    LhForwardImage  imager(IMAGE_WIDTH, IMAGE_HEIGHT);

    const size_t BUF_SIZE = static_cast<size_t>(PACK_SIZE) * PACK_MAX_NUM;
    auto *frameDataBuf    = new uint8_t[BUF_SIZE];
    auto *intensityBuf    = new double[BUF_SIZE];
    auto *panSectorMem    = new uchar[BUF_SIZE];

    // --- extract -----------------------------------------------------------
    ifs.open(config.inputFile.toStdString(), std::ios::binary);

    int  savedCount   = 0;
    int  skippedCount = 0;
    bool frameReady;

    for (size_t selIdx = 0; selIdx < selected.size(); ++selIdx) {
        size_t   frameIdx   = selected[selIdx];
        uint64_t startOff   = allFrames[frameIdx].byteOffset;
        uint64_t endOff     = (frameIdx + 1 < allFrames.size())
                                  ? allFrames[frameIdx + 1].byteOffset
                                  : fileSize;
        uint64_t frameBytes = endOff - startOff;
        int      numPacks   = static_cast<int>(frameBytes / PACK_SIZE);

        if (numPacks <= 0) { ++skippedCount; continue; }

        // Read and assemble all packets for this frame
        ifs.seekg(startOff, std::ios::beg);
        frameReady = false;

        for (int p = 0; p < numPacks; ++p) {
            // Re-use intensityBuf as temporary packet read buffer
            ifs.read(reinterpret_cast<char *>(intensityBuf), PACK_SIZE);
            int ret = frameAssembler.writeOnePackData(
                reinterpret_cast<const uint8_t *>(intensityBuf));

            if (ret == LhForwardFrame::oneFrameOK) {
                frameReady = true;
                // Continue reading remaining packets even after oneFrameOK —
                // some frame types produce OK early and the rest is payload
                // we don't need. But break here since getOneFrame gives us
                // everything we need.
                break;
            }
            if (ret == LhForwardFrame::writePackErr) {
                // Bad packet header; continue with next packet
                continue;
            }
        }

        if (!frameReady) { ++skippedCount; continue; }

        // Retrieve assembled frame
        struct packageInfo frameInfo;
        memset(&frameInfo, 0, sizeof(frameInfo));
        memset(frameDataBuf, 0, BUF_SIZE);
        frameAssembler.getOneFrame(&frameInfo, frameDataBuf);

        // Generate QImage from frame data
        QImage img;

        if (frameInfo.packType == 0) {
            // Beam data — bilinear interpolation imaging
            double imageRes = 0.0;
            int panSize = (IMAGE_WIDTH > IMAGE_HEIGHT) ? IMAGE_WIDTH : IMAGE_HEIGHT;
            int lwidth  = IMAGE_WIDTH;
            int lheight = IMAGE_HEIGHT;
            if (static_cast<int>(frameInfo.horAngleReso) == 360) {
                lwidth  = panSize;
                lheight = panSize;
            }
            memset(panSectorMem, 0, static_cast<size_t>(panSize) * panSize);
            imager.generateForwardImage(frameInfo, frameDataBuf,
                                        panSectorMem, imageRes);
            img = QImage(panSectorMem, lwidth, lheight, QImage::Format_Grayscale8);

        } else if (frameInfo.packType == 2) {
            // Uncompressed raw intensity data
            imager.preprocessData(frameInfo, frameDataBuf, intensityBuf);
            int coef = (frameInfo.dataWidth == 0x00) ? 1 : 256;
            size_t pixelCount = static_cast<size_t>(frameInfo.numPerRow)
                                * frameInfo.dataHeight;
            for (size_t i = 0; i < pixelCount; ++i)
                frameDataBuf[i] = static_cast<uint8_t>(intensityBuf[i] / coef);
            img = QImage(frameDataBuf, frameInfo.numPerRow,
                         frameInfo.dataHeight, QImage::Format_Grayscale8);

        } else if (frameInfo.packType == 3) {
            // Compressed PNG
            img.loadFromData(frameDataBuf,
                             static_cast<int>(frameInfo.dataHeight), "png");

        } else if (frameInfo.packType == 4) {
            // Compressed JPG
            img.loadFromData(frameDataBuf,
                             static_cast<int>(frameInfo.dataHeight), "jpg");

        } else {
            ++skippedCount;
            continue;
        }

        if (img.isNull()) { ++skippedCount; continue; }

        // Vertical mirror (matches the main GUI application behaviour)
        img = img.mirrored(false, true);

        // Save PNG
        QString savePath = QString("%1/frame_%2.png")
                               .arg(config.outputDir)
                               .arg(savedCount++, 6, 10, QChar('0'));

        if (img.save(savePath, "PNG")) {
            std::cout << "[" << (selIdx + 1) << "/" << selected.size()
                      << "] " << savePath.toStdString() << std::endl;
        } else {
            err << "Error: failed to save " << savePath << "\n";
            ++skippedCount;
        }
    }

    ifs.close();

    // --- cleanup -----------------------------------------------------------
    delete[] frameDataBuf;
    delete[] intensityBuf;
    delete[] panSectorMem;

    // --- summary -----------------------------------------------------------
    std::cout << "\n========================================\n"
              << "Extraction complete.\n"
              << "  Saved:   " << savedCount << " frames\n"
              << "  Skipped: " << skippedCount << " frames\n"
              << "  Output:  " << config.outputDir.toStdString() << "\n"
              << "========================================\n";

    return 0;
}
