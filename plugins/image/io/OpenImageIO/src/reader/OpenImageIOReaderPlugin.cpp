#include "OpenImageIOReaderPlugin.hpp"
#include "OpenImageIOReaderProcess.hpp"
#include "OpenImageIOReaderDefinitions.hpp"

#include <OpenImageIO/imageio.h>

#include <boost/gil.hpp>
#include <boost/filesystem.hpp>

#include <memory>

namespace tuttle
{
namespace plugin
{
namespace openImageIO
{
namespace reader
{

namespace bfs = boost::filesystem;
using namespace boost::gil;

OpenImageIOReaderPlugin::OpenImageIOReaderPlugin(OfxImageEffectHandle handle)
    : ReaderPlugin(handle)
{
}

OpenImageIOReaderProcessParams OpenImageIOReaderPlugin::getProcessParams(const OfxTime time)
{
    OpenImageIOReaderProcessParams params;

    params._filepath = getAbsoluteFilenameAt(time);
    return params;
}

void OpenImageIOReaderPlugin::changedParam(const OFX::InstanceChangedArgs& args, const std::string& paramName)
{
    ReaderPlugin::changedParam(args, paramName);
}

bool OpenImageIOReaderPlugin::getRegionOfDefinition(const OFX::RegionOfDefinitionArguments& args, OfxRectD& rod)
{
    const std::string filename(getAbsoluteFilenameAt(args.time));

    if(!bfs::exists(filename))
    {
        BOOST_THROW_EXCEPTION(exception::FileInSequenceNotExist() << exception::user("OpenImageIO: Unable to open file")
                                                                  << exception::filename(filename));
    }

    std::unique_ptr<OIIO::ImageInput> in(OIIO::ImageInput::create(filename));
    if(!in)
    {
        BOOST_THROW_EXCEPTION(exception::File() << exception::user("OpenImageIO: Unable to open file")
                                                << exception::filename(filename));
    }
    OIIO::ImageSpec spec;

    if(!in->open(filename, spec))
    {
        BOOST_THROW_EXCEPTION(exception::Unknown() << exception::user("OIIO Reader: " + in->geterror())
                                                   << exception::filename(filename));
    }

    rod.x1 = 0;
    rod.x2 = spec.width * this->_clipDst->getPixelAspectRatio();
    rod.y1 = 0;
    rod.y2 = spec.height;

    in->close();
    return true;
}

void OpenImageIOReaderPlugin::getClipPreferences(OFX::ClipPreferencesSetter& clipPreferences)
{
    ReaderPlugin::getClipPreferences(clipPreferences);

    const std::string filename(getAbsoluteFirstFilename());

    if(!bfs::exists(filename))
    {
        BOOST_THROW_EXCEPTION(exception::FileInSequenceNotExist() << exception::user("OpenImageIO: Unable to open file")
                                                                  << exception::filename(filename));
    }

    // if no filename
    if(filename.size() == 0)
    {
        clipPreferences.setClipBitDepth(*this->_clipDst, OFX::eBitDepthFloat);
        clipPreferences.setClipComponents(*this->_clipDst, OFX::ePixelComponentRGBA);
        clipPreferences.setPixelAspectRatio(*this->_clipDst, 1.0);
        return;
    }

    OIIO::ImageInput* in = OIIO::ImageInput::create(filename).get();

    if(!in)
    {
        BOOST_THROW_EXCEPTION(exception::Unknown() << exception::user("OIIO Reader: " + in->geterror())
                                                   << exception::filename(filename));
    }

    OIIO::ImageSpec spec;
    if(!in->open(filename, spec))
    {
        BOOST_THROW_EXCEPTION(exception::Unknown() << exception::user("OIIO Reader: " + in->geterror())
                                                   << exception::filename(filename));
    }

    if(getExplicitBitDepthConversion() == eParamReaderBitDepthAuto)
    {
        OFX::EBitDepth bd = OFX::eBitDepthNone;
        switch(spec.format.basetype)
        {
            //			case TypeDesc::UCHAR:
            case OIIO::TypeDesc::UINT8:
            //			case TypeDesc::CHAR:
            case OIIO::TypeDesc::INT8:
                bd = OFX::eBitDepthUByte;
                break;
            case OIIO::TypeDesc::HALF:
            //			case TypeDesc::USHORT:
            case OIIO::TypeDesc::UINT16:
            //			case TypeDesc::SHORT:
            case OIIO::TypeDesc::INT16:
                bd = OFX::eBitDepthUShort;
                break;
            //			case TypeDesc::UINT:
            case OIIO::TypeDesc::UINT32:
            //			case TypeDesc::INT:
            case OIIO::TypeDesc::INT32:
            //			case TypeDesc::ULONGLONG:
            case OIIO::TypeDesc::UINT64:
            //			case TypeDesc::LONGLONG:
            case OIIO::TypeDesc::INT64:
            case OIIO::TypeDesc::FLOAT:
            case OIIO::TypeDesc::DOUBLE:
                bd = OFX::eBitDepthFloat;
                break;
            case OIIO::TypeDesc::STRING:
            case OIIO::TypeDesc::PTR:
            case OIIO::TypeDesc::LASTBASE:
            case OIIO::TypeDesc::UNKNOWN:
            case OIIO::TypeDesc::NONE:
            default:
            {
                in->close();
                BOOST_THROW_EXCEPTION(exception::ImageFormat() << exception::user("bad input format"));
            }
        }
        clipPreferences.setClipBitDepth(*this->_clipDst, bd);
    }

    if(getExplicitChannelConversion() == eParamReaderChannelAuto)
    {
        switch(spec.nchannels)
        {
            case 1:
                clipPreferences.setClipComponents(*this->_clipDst, OFX::ePixelComponentAlpha);
                break;
            case 3:
                if(OFX::getImageEffectHostDescription()->supportsPixelComponent(OFX::ePixelComponentRGB))
                {
                    clipPreferences.setClipComponents(*this->_clipDst, OFX::ePixelComponentRGB);
                }
                else
                {
                    clipPreferences.setClipComponents(*this->_clipDst, OFX::ePixelComponentRGBA);
                }
                break;
            case 4:
                clipPreferences.setClipComponents(*this->_clipDst, OFX::ePixelComponentRGBA);
                break;
            default:
                clipPreferences.setClipComponents(*this->_clipDst, OFX::ePixelComponentRGBA);
                break;
        }
    }

    const float par = spec.get_float_attribute("PixelAspectRatio", 1.0f);
    clipPreferences.setPixelAspectRatio(*this->_clipDst, par);
    in->close();
}

/**
 * @brief The overridden render function
 * @param[in]   args     Rendering parameters
 */
void OpenImageIOReaderPlugin::render(const OFX::RenderArguments& args)
{
    ReaderPlugin::render(args);

    doGilRender<OpenImageIOReaderProcess>(*this, args);
}
}
}
}
}
