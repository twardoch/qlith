// this_file: qlith-pro/src/mimetyperegistry.cpp
#include "qlith/mimetyperegistry.h"

#include "qlith/common.h"

//#include "ArchiveFactory.h"
//#include "MediaPlayer.h"

/*#include "config.h"
#include "MIMETypeRegistry.h"

#include "ArchiveFactory.h"
#include "MediaPlayer.h"
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/StringHash.h>*/

#include <qimagereader.h>
#include <qimagewriter.h>

static HashSet<String>* supportedImageResourceMIMETypes;
static HashSet<String>* supportedImageMIMETypes;
static HashSet<String>* supportedImageMIMETypesForEncoding;
static HashSet<String>* supportedJavaScriptMIMETypes;
static HashSet<String>* supportedNonImageMIMETypes;
static HashSet<String>* supportedMediaMIMETypes;

typedef HashMap<String, QVector<String>*/*, CaseFoldingHash*/> MediaMIMETypeMap;

static void initializeSupportedImageMIMETypes()
{
//#elif PLATFORM(QT)
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for (size_t i = 0; i < static_cast<size_t>(formats.size()); ++i) {
/*#if ENABLE(SVG)
        // Qt has support for SVG, but we want to use KSVG2
        if (formats.at(i).toLower().startsWith("svg"))
            continue;
#endif*/
        String mimeType = MIMETypeRegistry::getMIMETypeForExtension(formats.at(i).constData());
        if (!mimeType.isEmpty()) {
            supportedImageMIMETypes->insert(mimeType);
            supportedImageResourceMIMETypes->insert(mimeType);
        }
    }
/*#else
    // assume that all implementations at least support the following standard
    // image types:
    static const char* types[] = {
        "image/jpeg",
        "image/png",
        "image/gif",
        "image/bmp",
        "image/vnd.microsoft.icon",    // ico
        "image/x-icon",    // ico
        "image/x-xbitmap"  // xbm
    };
    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
        supportedImageMIMETypes->add(types[i]);
        supportedImageResourceMIMETypes->add(types[i]);
    }
#endif*/
}

static void initializeSupportedImageMIMETypesForEncoding()
{
    supportedImageMIMETypesForEncoding = new HashSet<String>;

/*#if PLATFORM(CG)
#if PLATFORM(MAC)
    RetainPtr<CFArrayRef> supportedTypes(AdoptCF, CGImageDestinationCopyTypeIdentifiers());
    CFIndex count = CFArrayGetCount(supportedTypes.get());
    for (CFIndex i = 0; i < count; i++) {
        RetainPtr<CFStringRef> supportedType(AdoptCF, reinterpret_cast<CFStringRef>(CFArrayGetValueAtIndex(supportedTypes.get(), i)));
        String mimeType = MIMETypeForImageSourceType(supportedType.get());
        if (!mimeType.isEmpty())
            supportedImageMIMETypesForEncoding->add(mimeType);
    }
#else
    // FIXME: Add Windows support for all the supported UTI's when a way to convert from MIMEType to UTI reliably is found.
    // For now, only support PNG, JPEG and GIF.  See <rdar://problem/6095286>.
    supportedImageMIMETypesForEncoding->add("image/png");
    supportedImageMIMETypesForEncoding->add("image/jpeg");
    supportedImageMIMETypesForEncoding->add("image/gif");
#endif
#elif PLATFORM(QT)*/
    QList<QByteArray> formats = QImageWriter::supportedImageFormats();
    for (int i = 0; i < formats.size(); ++i) {
        String mimeType = MIMETypeRegistry::getMIMETypeForExtension(formats.at(i).constData());
        if (!mimeType.isEmpty())
            supportedImageMIMETypesForEncoding->insert(mimeType);
    }
/*#elif PLATFORM(GTK)
    supportedImageMIMETypesForEncoding->add("image/png");
    supportedImageMIMETypesForEncoding->add("image/jpeg");
    supportedImageMIMETypesForEncoding->add("image/tiff");
    supportedImageMIMETypesForEncoding->add("image/bmp");
    supportedImageMIMETypesForEncoding->add("image/ico");
#elif PLATFORM(CAIRO)
    supportedImageMIMETypesForEncoding->add("image/png");
#endif*/
}

static void initializeSupportedJavaScriptMIMETypes()
{
    /*
        Mozilla 1.8 and WinIE 7 both accept text/javascript and text/ecmascript.
        Mozilla 1.8 accepts application/javascript, application/ecmascript, and application/x-javascript, but WinIE 7 doesn't.
        WinIE 7 accepts text/javascript1.1 - text/javascript1.3, text/jscript, and text/livescript, but Mozilla 1.8 doesn't.
        Mozilla 1.8 allows leading and trailing whitespace, but WinIE 7 doesn't.
        Mozilla 1.8 and WinIE 7 both accept the empty string, but neither accept a whitespace-only string.
        We want to accept all the values that either of these browsers accept, but not other values.
     */
    static const char* types[] = {
        "text/javascript",
        "text/ecmascript",
        "application/javascript",
        "application/ecmascript",
        "application/x-javascript",
        "text/javascript1.1",
        "text/javascript1.2",
        "text/javascript1.3",
        "text/jscript",
        "text/livescript",
    };
    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i)
      supportedJavaScriptMIMETypes->insert(types[i]);
}

static void initializeSupportedNonImageMimeTypes()
{
    static const char* types[] = {
/*#if ENABLE(WML)
        "text/vnd.wap.wml",
        "application/vnd.wap.wmlc",
#endif*/
        "text/html",
        "text/xml",
        "text/xsl",
        "text/plain",
        "text/",
        "application/xml",
        "application/xhtml+xml",
        "application/vnd.wap.xhtml+xml",
        "application/rss+xml",
        "application/atom+xml",
        "application/json",
/*#if ENABLE(SVG)
        "image/svg+xml",
#endif
#if ENABLE(FTPDIR)
        "application/x-ftp-directory",
#endif*/
        "multipart/x-mixed-replace"
        // Note: ADDING a new type here will probably render it as HTML. This can
        // result in cross-site scripting.
    };
    /*COMPILE_ASSERT(sizeof(types) / sizeof(types[0]) <= 16,
                   nonimage_mime_types_must_be_less_than_or_equal_to_16);*/

    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); ++i)
        supportedNonImageMIMETypes->insert(types[i]);

    //ArchiveFactory::registerKnownArchiveMIMETypes();
}

static MediaMIMETypeMap& mediaMIMETypeMap()
{
    struct TypeExtensionPair {
        const char* type;
        const char* extension;
    };

    // A table of common media MIME types and file extenstions used when a platform's
    // specific MIME type lookup doesn't have a match for a media file extension.
    static const TypeExtensionPair pairs[] = {

        // Ogg
        { "application/ogg", "ogx" },
        { "audio/ogg", "ogg" },
        { "audio/ogg", "oga" },
        { "video/ogg", "ogv" },

        // Annodex
        { "application/annodex", "anx" },
        { "audio/annodex", "axa" },
        { "video/annodex", "axv" },
        { "audio/speex", "spx" },

        // WebM
        { "video/webm", "webm" },
        { "audio/webm", "webm" },

        // MPEG
        { "audio/mpeg", "m1a" },
        { "audio/mpeg", "m2a" },
        { "audio/mpeg", "m1s" },
        { "audio/mpeg", "mpa" },
        { "video/mpeg", "mpg" },
        { "video/mpeg", "m15" },
        { "video/mpeg", "m1s" },
        { "video/mpeg", "m1v" },
        { "video/mpeg", "m75" },
        { "video/mpeg", "mpa" },
        { "video/mpeg", "mpeg" },
        { "video/mpeg", "mpm" },
        { "video/mpeg", "mpv" },

        // MPEG playlist
        { "application/vnd.apple.mpegurl", "m3u8" },
        { "application/mpegurl", "m3u8" },
        { "application/x-mpegurl", "m3u8" },
        { "audio/mpegurl", "m3url" },
        { "audio/x-mpegurl", "m3url" },
        { "audio/mpegurl", "m3u" },
        { "audio/x-mpegurl", "m3u" },

        // MPEG-4
        { "video/x-m4v", "m4v" },
        { "audio/x-m4a", "m4a" },
        { "audio/x-m4b", "m4b" },
        { "audio/x-m4p", "m4p" },
        { "audio/mp4", "m4a" },

        // MP3
        { "audio/mp3", "mp3" },
        { "audio/x-mp3", "mp3" },
        { "audio/x-mpeg", "mp3" },

        // MPEG-2
        { "video/x-mpeg2", "mp2" },
        { "video/mpeg2", "vob" },
        { "video/mpeg2", "mod" },
        { "video/m2ts", "m2ts" },
        { "video/x-m2ts", "m2t" },
        { "video/x-m2ts", "ts" },

        // 3GP/3GP2
        { "audio/3gpp", "3gpp" },
        { "audio/3gpp2", "3g2" },
        { "application/x-mpeg", "amc" },

        // AAC
        { "audio/aac", "aac" },
        { "audio/aac", "adts" },
        { "audio/x-aac", "m4r" },

        // CoreAudio File
        { "audio/x-caf", "caf" },
        { "audio/x-gsm", "gsm" }
    };

    DEFINE_STATIC_LOCAL(MediaMIMETypeMap, mediaMIMETypeForExtensionMap, ());

    if (!mediaMIMETypeForExtensionMap.isEmpty())
        return mediaMIMETypeForExtensionMap;

    const unsigned numPairs = sizeof(pairs) / sizeof(pairs[0]);
    for (unsigned ndx = 0; ndx < numPairs; ++ndx) {

        if (mediaMIMETypeForExtensionMap.contains(pairs[ndx].extension))
            mediaMIMETypeForExtensionMap[(pairs[ndx].extension)]->append(pairs[ndx].type);
        else {
            QVector<String>* synonyms = new QVector<String>;

            // If there is a system specific type for this extension, add it as the first type so
            // getMediaMIMETypeForExtension will always return it.
            String systemType = MIMETypeRegistry::getMIMETypeForExtension(pairs[ndx].type);
            if (!systemType.isEmpty() && pairs[ndx].type != systemType)
                synonyms->append(systemType);
            synonyms->append(pairs[ndx].type);
            mediaMIMETypeForExtensionMap.insert(pairs[ndx].extension, synonyms);
        }
    }

    return mediaMIMETypeForExtensionMap;
}

String MIMETypeRegistry::getMediaMIMETypeForExtension(const String& ext)
{
    if (mediaMIMETypeMap().contains(ext))
        return (*mediaMIMETypeMap()[(ext)])[0];

    return String();
}

QVector<String> MIMETypeRegistry::getMediaMIMETypesForExtension(const String& ext)
{
    if (mediaMIMETypeMap().contains(ext))
        return *mediaMIMETypeMap()[(ext)];

    return QVector<String>();
}

static void initializeSupportedMediaMIMETypes()
{
    supportedMediaMIMETypes = new HashSet<String>;
/*#if ENABLE(VIDEO)
    MediaPlayer::getSupportedTypes(*supportedMediaMIMETypes);
#endif*/
}

static void initializeMIMETypeRegistry()
{
    supportedJavaScriptMIMETypes = new HashSet<String>;
    initializeSupportedJavaScriptMIMETypes();

    supportedNonImageMIMETypes = new HashSet<String>(*supportedJavaScriptMIMETypes);
    initializeSupportedNonImageMimeTypes();

    supportedImageResourceMIMETypes = new HashSet<String>;
    supportedImageMIMETypes = new HashSet<String>;
    initializeSupportedImageMIMETypes();
}

String MIMETypeRegistry::getMIMETypeForPath(const String& path)
{
    size_t pos = path.lastIndexOf('.');//path.reverseFind('.');
    if (pos != notFound) {
        // https://doc.qt.io/qt-5/qstring.html#mid
        String extension = path.mid(pos + 1); // path.subString(pos + 1);
        String result = getMIMETypeForExtension(extension);
        if (result.length())
            return result;
    }
    return "application/octet-stream";
}

bool MIMETypeRegistry::isSupportedImageMIMEType(const String& mimeType)
{
    if (mimeType.isEmpty())
        return false;
    if (!supportedImageMIMETypes)
        initializeMIMETypeRegistry();
    return supportedImageMIMETypes->contains(mimeType);
}

bool MIMETypeRegistry::isSupportedImageResourceMIMEType(const String& mimeType)
{
    if (mimeType.isEmpty())
        return false;
    if (!supportedImageResourceMIMETypes)
        initializeMIMETypeRegistry();
    return supportedImageResourceMIMETypes->contains(mimeType);
}

bool MIMETypeRegistry::isSupportedImageMIMETypeForEncoding(const String& mimeType)
{
    //ASSERT(isMainThread());

    if (mimeType.isEmpty())
        return false;
    if (!supportedImageMIMETypesForEncoding)
        initializeSupportedImageMIMETypesForEncoding();
    return supportedImageMIMETypesForEncoding->contains(mimeType);
}

bool MIMETypeRegistry::isSupportedJavaScriptMIMEType(const String& mimeType)
{
    if (mimeType.isEmpty())
        return false;
    if (!supportedJavaScriptMIMETypes)
        initializeMIMETypeRegistry();
    return supportedJavaScriptMIMETypes->contains(mimeType);
}

bool MIMETypeRegistry::isSupportedNonImageMIMEType(const String& mimeType)
{
    if (mimeType.isEmpty())
        return false;
    if (!supportedNonImageMIMETypes)
        initializeMIMETypeRegistry();
    return supportedNonImageMIMETypes->contains(mimeType);
}

bool MIMETypeRegistry::isSupportedMediaMIMEType(const String& mimeType)
{
    if (mimeType.isEmpty())
        return false;
    if (!supportedMediaMIMETypes)
        initializeSupportedMediaMIMETypes();
    return supportedMediaMIMETypes->contains(mimeType);
}

bool MIMETypeRegistry::isJavaAppletMIMEType(const String& mimeType)
{
    // Since this set is very limited and is likely to remain so we won't bother with the overhead
    // of using a hash set.
    // Any of the MIME types below may be followed by any number of specific versions of the JVM,
    // which is why we use startsWith()
    return mimeType.startsWith("application/x-java-applet")//, false)
        || mimeType.startsWith("application/x-java-bean")//, false)
        || mimeType.startsWith("application/x-java-vm");//, false);
}

HashSet<String>& MIMETypeRegistry::getSupportedImageMIMETypes()
{
    if (!supportedImageMIMETypes)
        initializeMIMETypeRegistry();
    return *supportedImageMIMETypes;
}

HashSet<String>& MIMETypeRegistry::getSupportedImageResourceMIMETypes()
{
    if (!supportedImageResourceMIMETypes)
        initializeMIMETypeRegistry();
    return *supportedImageResourceMIMETypes;
}

HashSet<String>& MIMETypeRegistry::getSupportedImageMIMETypesForEncoding()
{
    if (!supportedImageMIMETypesForEncoding)
        initializeSupportedImageMIMETypesForEncoding();
    return *supportedImageMIMETypesForEncoding;
}

HashSet<String>& MIMETypeRegistry::getSupportedNonImageMIMETypes()
{
    if (!supportedNonImageMIMETypes)
        initializeMIMETypeRegistry();
    return *supportedNonImageMIMETypes;
}

HashSet<String>& MIMETypeRegistry::getSupportedMediaMIMETypes()
{
    if (!supportedMediaMIMETypes)
        initializeSupportedMediaMIMETypes();
    return *supportedMediaMIMETypes;
}

const String& defaultMIMEType()
{
    DEFINE_STATIC_LOCAL(const String, defaultMIMEType, ("application/octet-stream"));
    return defaultMIMEType;
}
