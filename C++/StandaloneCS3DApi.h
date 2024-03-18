//***************************************************************************************/
//
// File name: StandaloneCS3DApi.h
//
// Synopsis:  Contains the definitions of stub classes and structures to make the 
//            Chromasens_3DPIXA_M10PP3 example work without having the CS3D API installed. 
//            This is done by loading the expected output of the CS3D API into 
//            the application.
//
// Copyright © 1992-2024 Zebra Technologies Corp. and/or its affiliates
// All Rights Reserved

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////
// Chromasens CS3D API enumeration redefinition.
//////////////////////////////////////////////////////////////////////////
enum outImgType
   {
   IMG_OUT_DISP,
   IMG_OUT_GRAY,
   IMG_OUT_BGRA
   };

enum channelOrder
   {
   CO_BGRA,
   CO_GRAY
   };

//////////////////////////////////////////////////////////////////////////
// Class that loads the output data from a Chromasens CS3D API output
// that was saved into an AVI.
//////////////////////////////////////////////////////////////////////////
class CStandalone3DOutput
   {
   public:
      // Constructor. Allocates the emulated digitizer for the outputs.
      CStandalone3DOutput(MIL_CONST_TEXT_PTR FilePathWithoutSuffix, MIL_CONST_TEXT_PTR Suffix, MIL_INT Attribute)
         {
         MIL_TEXT_CHAR FilePath[MAX_PATH];
         MosSprintf(FilePath, MAX_PATH, MIL_TEXT("%s_%s.avi"), FilePathWithoutSuffix, Suffix);
         MdigAlloc(M_DEFAULT_HOST, M_DEFAULT, FilePath, M_DEFAULT, &m_MilDigitizer);
         MdigInquire(m_MilDigitizer, M_SIZE_X, &m_ImageSizeX);
         MdigInquire(m_MilDigitizer, M_SIZE_Y, &m_ImageSizeY);
         MdigInquire(m_MilDigitizer, M_SIZE_BAND, &m_ImageSizeBand);
         MdigInquire(m_MilDigitizer, M_TYPE, &m_ImageType);
         }

      // Destructor. Frees the emulated digitizer.
      virtual ~CStandalone3DOutput()
         {
         MdigFree(m_MilDigitizer);
         }

      // Function that returns the output image information.
      void getDestImgInfo(int &width,int &height,int &channelCount,unsigned long long &sizeInByte)
         {
         width = (int)m_ImageSizeX;
         height = (int)m_ImageSizeY;
         channelCount = (int)m_ImageSizeBand;
         sizeInByte = width * height * (int)m_ImageSizeBand;
         }

      // Function that loads the image into the MIL buffer data pointer provided.
      int getLastImage(void** imgPtr, int linePitch)
         {
         MIL_INT Attribute = m_ImageSizeBand == 3 ? M_BGR32 + M_PACKED : M_NULL;
         MIL_ID MilStandaloneOutputImage = MbufCreateColor(M_DEFAULT_HOST,
                                                           m_ImageSizeBand,
                                                           m_ImageSizeX,
                                                           m_ImageSizeY,
                                                           m_ImageType,
                                                           M_IMAGE + M_GRAB + Attribute,
                                                           M_HOST_ADDRESS + M_PITCH_BYTE,
                                                           linePitch,
                                                           imgPtr,
                                                           M_NULL); 
         MdigGrab(m_MilDigitizer, MilStandaloneOutputImage);
         MbufFree(MilStandaloneOutputImage);
         return 0;
         }

   private:
      MIL_INT m_ImageSizeX;
      MIL_INT m_ImageSizeY;
      MIL_INT m_ImageSizeBand;
      MIL_INT m_ImageType;
      MIL_ID m_MilDigitizer;
   };

//////////////////////////////////////////////////////////////////////////
// Standalone version of the Chromasens config3DApi object that contains
// the parameters of the config.ini file.
//////////////////////////////////////////////////////////////////////////
struct config3DApi
   {
   int imgWidth;
   int imgHeight;
   int windowType;

   int intensityChannelUsed;
   int numChannelsUsedForCalculation;

   //Calibration parameters.
   double resolutionX;
   double resolutionY;

   //Disparity calculation.
   double mingw;
   double maxgw;
   double minStdDevA;
   double minKkf;
   double maxConsistent;
   int	dStart;
   int	dEnd;
   int disp;
   double dY;
   double dispThreshErr;

   int oriImgWidth;
   int oriImgHeight;
   };

//////////////////////////////////////////////////////////////////////////
// Hard coded configuration values for the standalone mode of the example.
//////////////////////////////////////////////////////////////////////////
static const double COMPACT_resolutionX = 0.015;
static const double COMPACT_resolutionY = 0.015;
static const double COMPACT_minGrayRefMmValue = 126.238533;
static const double COMPACT_maxGrayRefMmValue = 125.361740;

static const int COMPACT_imgWidth = 2968;
static const int COMPACT_imgHeight = 4500;
static const int COMPACT_windowType = 4;

static const int COMPACT_intensityChannelUsed = 1;
static const int COMPACT_numChannelsUsedForCalculation = 3;

static const double COMPACT_mingw = 1.0;
static const double COMPACT_maxgw = 255.0;
static const double COMPACT_minStdDevA = 2.0;
static const double COMPACT_minKkf = 0.2;
static const double COMPACT_maxConsistent = 1.0;
static const int    COMPACT_dStart = -17;
static const int    COMPACT_dEnd = -5;
static const int    COMPACT_disp = 1;
static const double COMPACT_dY = 0.0;
static const double COMPACT_dispThreshErr= 0.0;

static const int COMPACT_oriImgWidth = 2968;
static const int COMPACT_oriImgHeight = 4500;

//////////////////////////////////////////////////////////////////////////
// Standalone version of the real Chromasens I3DApi interface that loads
// the output of Chromasens CS3D API calculation.
//////////////////////////////////////////////////////////////////////////
class I3DApi
   {
   public:
      // Constructor.
      I3DApi(MIL_CONST_TEXT_PTR PrefixFile)
         : m_ColorOutput(PrefixFile, MIL_TEXT("Color"), M_BGR32 + M_PACKED),
           m_DisparityOutput(PrefixFile, MIL_TEXT("Disparity"), M_NULL)
         {
         // Setup the config.
         m_Config3DApi.dEnd                          = COMPACT_dEnd;
         m_Config3DApi.imgWidth                      = COMPACT_imgWidth;
         m_Config3DApi.imgHeight                     = COMPACT_imgHeight;
         m_Config3DApi.windowType                    = COMPACT_windowType;
         m_Config3DApi.intensityChannelUsed          = COMPACT_intensityChannelUsed;
         m_Config3DApi.numChannelsUsedForCalculation = COMPACT_numChannelsUsedForCalculation;
         m_Config3DApi.mingw                         = COMPACT_mingw;
         m_Config3DApi.maxgw                         = COMPACT_maxgw;
         m_Config3DApi.minStdDevA                    = COMPACT_minStdDevA;
         m_Config3DApi.minKkf                        = COMPACT_minKkf;
         m_Config3DApi.maxConsistent                 = COMPACT_maxConsistent;
         m_Config3DApi.dStart                        = COMPACT_dStart;
         m_Config3DApi.dEnd                          = COMPACT_dEnd;
         m_Config3DApi.disp                          = COMPACT_disp;
         m_Config3DApi.dY                            = COMPACT_dY;
         m_Config3DApi.dispThreshErr                 = COMPACT_dispThreshErr;
         m_Config3DApi.oriImgWidth                   = COMPACT_oriImgWidth;
         m_Config3DApi.oriImgHeight                  = COMPACT_oriImgHeight;

         m_Config3DApi.resolutionX                   =COMPACT_resolutionX;
         m_Config3DApi.resolutionY                   =COMPACT_resolutionY;
         }

      // Destructor.
      virtual ~I3DApi(){};

      // Stub functions that are not being used.
      int initialize(config3DApi *newCfg){return 0;}
      int start(void){return 0;}
      void stopBlocking(){};
      long getNextImgBlocking(void){return 0;}
      int setSrcImgPtr(int camNr,char * p){return 0;}
      int setSrcImgLoaded(int cam){return 0;}
      int setSrcImgChannelOrder(int camNr,channelOrder status){return 0;}
      int setSrcImgInfo(int camNr, int width, int height, int channelCount, int bpp, int linePitch,long sizeInByte){return 0;}

      // Function that convert the gray values to height data. This function is an linear approximation and 
      // does not give the exact same result as the CS-3d api.
      int grayToMm(float &distInMm, unsigned short grayValue)
         {
         const float Slope = (float)((COMPACT_maxGrayRefMmValue - COMPACT_minGrayRefMmValue) / (COMPACT_dEnd - COMPACT_dStart));
         const float WorkingDistance = (float)(COMPACT_minGrayRefMmValue - COMPACT_dStart * Slope);
         const float minGrayMmValue = m_Config3DApi.dStart * Slope + WorkingDistance;
         const float maxGrayMmValue = m_Config3DApi.dEnd * Slope + WorkingDistance;
         const float HeightRange = maxGrayMmValue - minGrayMmValue;
         distInMm = minGrayMmValue + (grayValue - 1.0f) / (65535.0f - 1.0f) * HeightRange;
         return 0;
         }
      
      // Function to get the information of the output AVI.
      void getDestImgInfo(outImgType imgType,int &width,int &height,int &channelCount,unsigned long long &sizeInByte)
         {
         switch (imgType)
            {
         case IMG_OUT_BGRA:
            m_ColorOutput.getDestImgInfo(width, height, channelCount, sizeInByte);
            break;
         case IMG_OUT_DISP:
         default:
            m_DisparityOutput.getDestImgInfo(width, height, channelCount, sizeInByte);
            break;
            }
         };

      // Function to get the next image of the output AVI.
      int getLastImage(void ** imgPtr, int linePitch, outImgType type)
         {
         switch (type)
            {
         case IMG_OUT_BGRA:
            m_ColorOutput.getLastImage(imgPtr, linePitch);
            break;
         case IMG_OUT_DISP:
         default:
            m_DisparityOutput.getLastImage(imgPtr, linePitch);
            break;
            }
         return 0;
         }

      // Function to return the configuration object.
      config3DApi* getConfig() {return &m_Config3DApi;}

   private:
      CStandalone3DOutput m_DisparityOutput;
      CStandalone3DOutput m_ColorOutput;

      config3DApi m_Config3DApi;
   };
