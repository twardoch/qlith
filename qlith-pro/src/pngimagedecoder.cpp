// this_file: qlith-pro/src/pngimagedecoder.cpp
#include "qlith/pngimagedecoder.h"

#include "qlith/common.h"
#include <png.h>

#if defined(PNG_LIBPNG_VER_MAJOR) && defined(PNG_LIBPNG_VER_MINOR) && (PNG_LIBPNG_VER_MAJOR > 1 || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4))
#define JMPBUF(png_ptr) png_jmpbuf(png_ptr)
#else
#define JMPBUF(png_ptr) png_ptr->jmpbuf
#endif

// Gamma constants.
const double cMaxGamma = 21474.83;
const double cDefaultGamma = 2.2;
const double cInverseGamma = 0.45455;

// Protect against large PNGs. See Mozilla's bug #251381 for more info.
const unsigned long cMaxPNGSize = 1000000UL;

// Called if the decoding of the image fails.
static void PNGAPI decodingFailed(png_structp png, png_const_charp)
{
    longjmp(JMPBUF(png), 1);
}

// Callbacks given to the read struct.  The first is for warnings (we want to
// treat a particular warning as an error, which is why we have to register this
// callback).
static void PNGAPI decodingWarning(png_structp png, png_const_charp warningMsg)
{
    // Mozilla did this, so we will too.
    // Convert a tRNS warning to be an error (see
    // http://bugzilla.mozilla.org/show_bug.cgi?id=251381 )
    if (!strncmp(warningMsg, "Missing PLTE before tRNS", 24))
        png_error(png, warningMsg);
}

// Called when we have obtained the header information (including the size).
static void PNGAPI headerAvailable(png_structp png, png_infop)
{
    static_cast<PNGImageDecoder*>(png_get_progressive_ptr(png))->headerAvailable();
}

// Called when a row is ready.
static void PNGAPI rowAvailable(png_structp png, png_bytep rowBuffer, png_uint_32 rowIndex, int interlacePass)
{
    static_cast<PNGImageDecoder*>(png_get_progressive_ptr(png))->rowAvailable(rowBuffer, rowIndex, interlacePass);
}

// Called when we have completely finished decoding the image.
static void PNGAPI pngComplete(png_structp png, png_infop)
{
    static_cast<PNGImageDecoder*>(png_get_progressive_ptr(png))->pngComplete();
}

class PNGImageReader
{
public:
    PNGImageReader(PNGImageDecoder* decoder)
        : m_readOffset(0)
        , m_decodingSizeOnly(false)
        , m_interlaceBuffer(0)
        , m_hasAlpha(false)
        , m_currentBufferSize(0)
    {
        m_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, decodingFailed, decodingWarning);
        m_info = png_create_info_struct(m_png);
        png_set_progressive_read_fn(m_png, decoder, headerAvailable, rowAvailable, pngComplete);
    }

    ~PNGImageReader()
    {
        close();
    }

    void close()
    {
        if (m_png && m_info)
            // This will zero the pointers.
            png_destroy_read_struct(&m_png, &m_info, 0);
        delete[] m_interlaceBuffer;
        m_interlaceBuffer = 0;
        m_readOffset = 0;
    }

    unsigned currentBufferSize() const { return m_currentBufferSize; }

    bool decode(const SharedBuffer& data, bool sizeOnly)
    {
        m_decodingSizeOnly = sizeOnly;
        PNGImageDecoder* decoder = static_cast<PNGImageDecoder*>(png_get_progressive_ptr(m_png));

        // We need to do the setjmp here. Otherwise bad things will happen.
        if (setjmp(JMPBUF(m_png)))
            return decoder->setFailed();

        const char* segment;
        while (unsigned segmentLength = data.getSomeData(segment, m_readOffset)) {
            m_readOffset += segmentLength;
            m_currentBufferSize = m_readOffset;
            png_process_data(m_png, m_info, reinterpret_cast<png_bytep>(const_cast<char*>(segment)), segmentLength);
            // We explicitly specify the superclass isSizeAvailable() because we
            // merely want to check if we've managed to set the size, not
            // (recursively) trigger additional decoding if we haven't.
            if (sizeOnly ? decoder->ImageDecoder::isSizeAvailable() : decoder->isComplete())
                return true;
        }
        return false;
    }

    bool decodingSizeOnly() const { return m_decodingSizeOnly; }
    png_structp pngPtr() const { return m_png; }
    png_infop infoPtr() const { return m_info; }
    png_bytep interlaceBuffer() const { return m_interlaceBuffer; }
    bool hasAlpha() const { return m_hasAlpha; }

    void setReadOffset(unsigned offset) { m_readOffset = offset; }
    void setHasAlpha(bool b) { m_hasAlpha = b; }

    void createInterlaceBuffer(int size) { m_interlaceBuffer = new png_byte[size]; }

private:
    unsigned m_readOffset;
    bool m_decodingSizeOnly;
    png_structp m_png;
    png_infop m_info;
    png_bytep m_interlaceBuffer;
    bool m_hasAlpha;
    unsigned m_currentBufferSize;
};

PNGImageDecoder::PNGImageDecoder(bool premultiplyAlpha)
    : ImageDecoder(premultiplyAlpha)
    , m_doNothingOnFailure(false)
{
}

PNGImageDecoder::~PNGImageDecoder()
{
}

bool PNGImageDecoder::isSizeAvailable()
{
    if (!ImageDecoder::isSizeAvailable())
         decode(true);

    return ImageDecoder::isSizeAvailable();
}

bool PNGImageDecoder::setSize(unsigned width, unsigned height)
{
    if (!ImageDecoder::setSize(width, height))
        return false;

    prepareScaleDataIfNecessary();
    return true;
}

RGBA32Buffer* PNGImageDecoder::frameBufferAtIndex(size_t index)
{
    if (index)
        return 0;

    if (m_frameBufferCache.isEmpty()) {
        m_frameBufferCache.resize(1);
        m_frameBufferCache[0].setPremultiplyAlpha(m_premultiplyAlpha);
    }

    RGBA32Buffer& frame = m_frameBufferCache[0];
    if (frame.status() != RGBA32Buffer::FrameComplete)
        decode(false);
    return &frame;
}

bool PNGImageDecoder::setFailed()
{
    if (m_doNothingOnFailure)
        return false;
    m_reader.clear();
    return ImageDecoder::setFailed();
}

void PNGImageDecoder::headerAvailable()
{
    png_structp png = m_reader->pngPtr();
    png_infop info = m_reader->infoPtr();
    png_uint_32 width = png->width;
    png_uint_32 height = png->height;

    // Protect against large images.
    if (png->width > cMaxPNGSize || png->height > cMaxPNGSize) {
        longjmp(JMPBUF(png), 1);
        return;
    }

    // We can fill in the size now that the header is available.  Avoid memory
    // corruption issues by neutering setFailed() during this call; if we don't
    // do this, failures will cause |m_reader| to be deleted, and our jmpbuf
    // will cease to exist.  Note that we'll still properly set the failure flag
    // in this case as soon as we longjmp().
    m_doNothingOnFailure = true;
    bool result = setSize(width, height);
    m_doNothingOnFailure = false;
    if (!result) {
        longjmp(JMPBUF(png), 1);
        return;
    }

    int bitDepth, colorType, interlaceType, compressionType, filterType, channels;
    png_get_IHDR(png, info, &width, &height, &bitDepth, &colorType, &interlaceType, &compressionType, &filterType);

    // The options we set here match what Mozilla does.

    // Expand to ensure we use 24-bit for RGB and 32-bit for RGBA.
    if (colorType == PNG_COLOR_TYPE_PALETTE || (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8))
        png_set_expand(png);

    png_bytep trns = 0;
    int trnsCount = 0;
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_get_tRNS(png, info, &trns, &trnsCount, 0);
        png_set_expand(png);
    }

    if (bitDepth == 16)
        png_set_strip_16(png);

    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    // Deal with gamma and keep it under our control.
    double gamma;
    if (png_get_gAMA(png, info, &gamma)) {
        if ((gamma <= 0.0) || (gamma > cMaxGamma)) {
            gamma = cInverseGamma;
            png_set_gAMA(png, info, gamma);
        }
        png_set_gamma(png, cDefaultGamma, gamma);
    } else
        png_set_gamma(png, cDefaultGamma, cInverseGamma);

    // Tell libpng to send us rows for interlaced pngs.
    if (interlaceType == PNG_INTERLACE_ADAM7)
        png_set_interlace_handling(png);

    // Update our info now.
    png_read_update_info(png, info);
    channels = png_get_channels(png, info);
    ASSERT(channels == 3 || channels == 4);

    m_reader->setHasAlpha(channels == 4);

    if (m_reader->decodingSizeOnly()) {
        // If we only needed the size, halt the reader.
        m_reader->setReadOffset(m_reader->currentBufferSize() - png->buffer_size);
        png->buffer_size = 0;
    }
}

void PNGImageDecoder::rowAvailable(unsigned char* rowBuffer, unsigned rowIndex, int interlacePass)
{
    if (m_frameBufferCache.isEmpty())
        return;

    // Initialize the framebuffer if needed.
    RGBA32Buffer& buffer = m_frameBufferCache[0];
    if (buffer.status() == RGBA32Buffer::FrameEmpty) {
        if (!buffer.setSize(scaledSize().width(), scaledSize().height())) {
            longjmp(JMPBUF(m_reader->pngPtr()), 1);
            return;
        }
        buffer.setStatus(RGBA32Buffer::FramePartial);
        buffer.setHasAlpha(false);

        // For PNGs, the frame always fills the entire image.
        buffer.setRect(IntRect(IntPoint(), size()));

        if (m_reader->pngPtr()->interlaced)
            m_reader->createInterlaceBuffer((m_reader->hasAlpha() ? 4 : 3) * size().width() * size().height());
    }

    if (!rowBuffer)
        return;

    // libpng comments (pasted in here to explain what follows)
    /*
     * this function is called for every row in the image.  If the
     * image is interlacing, and you turned on the interlace handler,
     * this function will be called for every row in every pass.
     * Some of these rows will not be changed from the previous pass.
     * When the row is not changed, the new_row variable will be NULL.
     * The rows and passes are called in order, so you don't really
     * need the row_num and pass, but I'm supplying them because it
     * may make your life easier.
     *
     * For the non-NULL rows of interlaced images, you must call
     * png_progressive_combine_row() passing in the row and the
     * old row.  You can call this function for NULL rows (it will
     * just return) and for non-interlaced images (it just does the
     * memcpy for you) if it will make the code easier.  Thus, you
     * can just do this for all cases:
     *
     *    png_progressive_combine_row(png_ptr, old_row, new_row);
     *
     * where old_row is what was displayed for previous rows.  Note
     * that the first pass (pass == 0 really) will completely cover
     * the old row, so the rows do not have to be initialized.  After
     * the first pass (and only for interlaced images), you will have
     * to pass the current row, and the function will combine the
     * old row and the new row.
     */

    png_structp png = m_reader->pngPtr();
    bool hasAlpha = m_reader->hasAlpha();
    unsigned colorChannels = hasAlpha ? 4 : 3;
    png_bytep row;
    png_bytep interlaceBuffer = m_reader->interlaceBuffer();
    if (interlaceBuffer) {
        row = interlaceBuffer + (rowIndex * colorChannels * size().width());
        png_progressive_combine_row(png, row, rowBuffer);
    } else
        row = rowBuffer;

    // Copy the data into our buffer.
    int width = scaledSize().width();
    int destY = scaledY(rowIndex);

    // Check that the row is within the image bounds. LibPNG may supply an extra row.
    if (destY < 0 || destY >= scaledSize().height())
        return;
    bool sawAlpha = buffer.hasAlpha();
    for (int x = 0; x < width; ++x) {
        png_bytep pixel = row + (m_scaled ? m_scaledColumns[x] : x) * colorChannels;
        unsigned alpha = hasAlpha ? pixel[3] : 255;
        buffer.setRGBA(x, destY, pixel[0], pixel[1], pixel[2], alpha);
        if (!sawAlpha && alpha < 255) {
            sawAlpha = true;
            buffer.setHasAlpha(true);
        }
    }
}

void PNGImageDecoder::pngComplete()
{
    if (!m_frameBufferCache.isEmpty())
        m_frameBufferCache.first().setStatus(RGBA32Buffer::FrameComplete);
}

void PNGImageDecoder::decode(bool onlySize)
{
    if (failed())
        return;

    if (!m_reader)
        m_reader.set(new PNGImageReader(this));

    // If we couldn't decode the image but we've received all the data, decoding
    // has failed.
    if (!m_reader->decode(*m_data, onlySize) && isAllDataReceived())
        setFailed();
    // If we're done decoding the image, we don't need the PNGImageReader
    // anymore.  (If we failed, |m_reader| has already been cleared.)
    else if (isComplete())
        m_reader.clear();
}
