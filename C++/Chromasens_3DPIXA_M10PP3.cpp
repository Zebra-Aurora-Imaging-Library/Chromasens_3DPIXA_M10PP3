//***************************************************************************************/
//
// File name: Chromasens_3DPIXA_M10PP3.cpp
//
// Synopsis:  This example shows how to use the 3DPIXA camera and its CS-3D API
//            in conjunction with MIL to solve surface inspection applications.
//            See the PrintHeader() function below for detailed description.
//
// Copyright © 1992-2024 Zebra Technologies Corp. and/or its affiliates
// All Rights Reserved

#include <mil.h>
#include "MdispD3D.h"

#define USE_CS3D_API 0

#if USE_CS3D_API 
   
   #if defined(WIN32)
      #error "Chromasens CS3D api cannot be used in a 32-bit application."
   #endif
   
   #include "C:\\Program Files\\Chromasens\\3D\\3dapi\\includes\\CS3DApiDll.h"
   using namespace CS3D;
   
   static MIL_CONST_TEXT_PTR CS3D_DLL_PATH  = MIL_TEXT("C:\\Program Files\\Chromasens\\3D\\dlls");
#else
   #include "StandaloneCS3DApi.h"
#endif

///***************************************************************************
// Example description.
///***************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("Chromasens_3DPIXA_M10PP3\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows how to use the 3DPIXA camera and its CS-3D API\n")
             MIL_TEXT("in conjunction with MIL to develop surface inspection applications.\n")
             MIL_TEXT("The example is able to run without the Chromasens CS3D API by\n")
             MIL_TEXT("loading from file the expected output images.\n\n")
             
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, calibration, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Camera source parameters.
// 
// The required Matrox board-specific DCF files for the Chromasens 3DPIXA camera 
// are available from the Interfacing Cameras section under the Support section of
// www.matrox.com/imaging 
//
//*****************************************************************************

static MIL_CONST_TEXT_PTR SOLIOS_DCF_PATH = M_NULL;
static MIL_CONST_TEXT_PTR RADIENT_DCF_PATH = M_NULL;

static MIL_CONST_TEXT_PTR COMPACT_STANDALONE_IMAGE_PATH         = M_IMAGE_PATH MIL_TEXT("Chromasens_3DPIXA_M10PP3\\Compact.avi");
static MIL_CONST_TEXT_PTR COMPACT_STANDALONE_OUTPUT_IMAGE_PATH  = M_IMAGE_PATH MIL_TEXT("Chromasens_3DPIXA_M10PP3\\CompactOutput");
static MIL_CONST_TEXT_PTR COMPACT_DATA_FORMAT[3]         = {COMPACT_STANDALONE_IMAGE_PATH, SOLIOS_DCF_PATH, RADIENT_DCF_PATH};

static char*              COMPACT_CONFIG_FILE       = "CompactConfig.ini";

static MIL_CONST_TEXT_PTR SYSTEM_DESCRIPTOR[3] = {M_SYSTEM_HOST, M_SYSTEM_SOLIOS, M_SYSTEM_RADIENT};
static const MIL_INT SYSTEM_TO_USE = 0;

//*****************************************************************************
// Useful struct.
//*****************************************************************************
struct SGrabStruct
   {
   MIL_ID MilDisplay;
   MIL_ID MilDigitizer;
   MIL_ID MilGrabImage;
   };

//*****************************************************************************
// Example prototypes.
//*****************************************************************************
void ParticleBoardInspectionExample(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilDigitizer, MIL_ID MilGrabImage, I3DApi* p3DApi, config3DApi *pConfig);
void SandPaperInspectionExample(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilDigitizer, MIL_ID MilGrabImage, I3DApi* p3DApi, config3DApi *pConfig);

//*****************************************************************************
// General function prototypes.
//*****************************************************************************
// 3D API functions.
bool AccessDll(HINSTANCE* phDll, I3DApi** pp3DApi, config3DApi **ppConfig, MIL_CONST_TEXT_PTR DllName, char* FactoryProcName, char* ConfigFile);
void FreeDll(HINSTANCE* phDll, I3DApi** pp3DApi);
bool Initialize3DApi(I3DApi* p3DApi,
                     config3DApi *pConfig,
                     MIL_ID MilSystem,
                     MIL_ID* pMilSrcImages,
                     MIL_INT NbSrcImage,
                     MIL_ID* pMilDisparityImage,
                     MIL_ID* pMilRectifiedImage,
                     MIL_INT* pWorkSizeX,
                     MIL_INT* pWorkSizeY);

// Depth map processing functions.
void FillHolesAndSmooth(MIL_ID MilDisplay, MIL_ID MilDepthMap, MIL_ID MilFilledHolesDepthMap, MIL_INT FilterSize);
void CorrectHorizontalCurve(MIL_ID MilDepthMap, MIL_INT ChildOffsetY, MIL_INT ChildSizeY);

// Utility functions.
void GrabImage(I3DApi* p3DApi,
               MIL_ID* pMilDisplays,
               MIL_ID* pMilDigitizers,
               MIL_ID* pMilGrabImages,
               MIL_INT NbCamera);

void Calculate3D(I3DApi* p3DApi,
                 MIL_ID* pMilDisplays,
                 MIL_ID* pMilSrcImages,
                 MIL_INT NbSrcImage,
                 MIL_ID MilDisparityImage,
                 MIL_ID MilrectifiedImage,
                 MIL_ID MilCorrectedWorkDepthMap,
                 MIL_ID MilCorrectedWorkColorMap);
MIL_INT GenAverageCircleKernel(MIL_ID MilAverageKernel);
MIL_DOUBLE CalibrateDepthMap(MIL_ID MilDepthMap, I3DApi* p3DApi, config3DApi *pConfig, MIL_DOUBLE XYMultFactor, MIL_DOUBLE ZMultFactor);
void ShowImage(MIL_ID MilDisplay, MIL_ID MilImage, bool Autoscale);
MIL_UINT32 MFTYPE StartScan(void *UserDataPtr);

bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR  FileName);

//*****************************************************************************
// Useful defines.
//*****************************************************************************
static const MIL_INT BORDER_SIZE_X = 64;
static const MIL_DOUBLE DISPLAY_ZOOM_FACTOR = 0.125;
static const MIL_INT WINDOWS_OFFSET_X = 15;

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the MIL objects.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_ID MilSystem      = MsysAlloc(M_DEFAULT, SYSTEM_DESCRIPTOR[SYSTEM_TO_USE], M_DEFAULT, M_DEFAULT, M_NULL);  
   MIL_ID pMilDisplay[2];
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &pMilDisplay[0]);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &pMilDisplay[1]);
   MdispZoom(pMilDisplay[0], DISPLAY_ZOOM_FACTOR, DISPLAY_ZOOM_FACTOR);
   MdispZoom(pMilDisplay[1], DISPLAY_ZOOM_FACTOR, DISPLAY_ZOOM_FACTOR);

   // Print Header.
   PrintHeader();

   // If the DCF file hasn't been specified.
   if(SYSTEM_TO_USE != 0 && COMPACT_DATA_FORMAT[SYSTEM_TO_USE] == NULL)
      {
      MosPrintf(MIL_TEXT("Please set a valid DCF path to run the example with a digitizer.\n")
                MIL_TEXT("The required Matrox board-specific DCF files for the Chromasens 3DPIXA\n") 
                MIL_TEXT("are available from the Interfacing Cameras section under the\n") 
                MIL_TEXT("Support section of www.matrox.com/imaging\n\n")
                MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }
   else
      {
      // Check if the footage is available when using emulated grab on host system.
      if(SYSTEM_TO_USE != 0 || CheckForRequiredMILFile(COMPACT_DATA_FORMAT[SYSTEM_TO_USE]))
         {
         // Allocate the digitizer and image for the compact Chromasens camera.
         MIL_ID pMilDigitizer[2];
         MdigAlloc(MilSystem, M_DEFAULT, COMPACT_DATA_FORMAT[SYSTEM_TO_USE], M_DEFAULT, &pMilDigitizer[0]);
         MIL_INT GrabImageSizeX = MdigInquire(pMilDigitizer[0], M_SIZE_X, M_NULL);
         MIL_INT GrabImageSizeY = MdigInquire(pMilDigitizer[0], M_SIZE_Y, M_NULL);
         MIL_ID pMilGrabImage[2];
         MbufAllocColor(MilSystem, 3, GrabImageSizeX, GrabImageSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP + M_BGR32 + M_GRAB, &pMilGrabImage[0]);

         // Get the path to the config file.
         char CompactConfigFilePath[MAX_PATH];
         char* MilPath;
         size_t Len;
         _dupenv_s(&MilPath, &Len, "MIL_PATH");
         sprintf_s(CompactConfigFilePath, MAX_PATH, "%s\\..\\..\\Images\\Chromasens_3DPIXA_M10PP3\\%s", MilPath, COMPACT_CONFIG_FILE);
         free(MilPath);

         // Allocate Chromasens 3DAPI for the compact Chromasens camera.
         HINSTANCE hDll;
         I3DApi* p3DApi = NULL;
         config3DApi *pConfig = NULL;

         if(AccessDll(&hDll, &p3DApi, &pConfig, MIL_TEXT("CS3DApi64.dll"), "CS3DApiCreate", CompactConfigFilePath))
            {
            // Set the image height according to the digitizer.
            pConfig->imgHeight = (int)GrabImageSizeY;
            pConfig->oriImgHeight = (int)GrabImageSizeY;

            // Run the particle board example
            ParticleBoardInspectionExample(MilSystem, pMilDisplay[0], pMilDigitizer[0], pMilGrabImage[0], p3DApi, pConfig);

            // Run the sand paper example.
            SandPaperInspectionExample(MilSystem, pMilDisplay[0], pMilDigitizer[0], pMilGrabImage[0], p3DApi, pConfig);
            }

         // Free the Chromasens 3dAPI.
         FreeDll(&hDll, &p3DApi);

         // Free the grab image.
         MbufFree(pMilGrabImage[0]);

         // Free the digitizers.
         MdigFree(pMilDigitizer[0]);
         }
      }

   // Free MIL objects.
   MdispFree(pMilDisplay[0]);
   MdispFree(pMilDisplay[1]);
   MsysFree(MilSystem);
   MappFree(MilApplication);
   }

//*****************************************************************************
// Particle board inspection example parameters.
//*****************************************************************************
static const MIL_INT D3D_DISPLAY_SIZE_X = 640;
static const MIL_INT D3D_DISPLAY_SIZE_Y = 480;
static const MIL_DOUBLE D3D_DISPLAY_SUBSAMPLING = 0.25;

static const MIL_DOUBLE PLANE_OUTLIER_DISTANCE_RANGE_FACTOR = 0.1;

static const MIL_INT HORIZONTAL_CURVE_CORRECTION_CHILD_OFFSET_Y = 0;
static const MIL_INT HORIZONTAL_CURVE_CORRECTION_CHILD_SIZE_Y = 1000;

static const MIL_DOUBLE DEFECT_THRESHOLD_HIGH = 0.096; // in mm
static const MIL_DOUBLE DEFECT_THRESHOLD_LOW = 0.048; // in mm

static const MIL_INT PARTICLEBOARD_KERNEL_SIZE = 51;

static const MIL_DOUBLE PARTICLEBOARD_Z_MULT_FACTOR = 8.333333;
//*****************************************************************************
// ParticleBoardInspectionExample.  
//*****************************************************************************
void ParticleBoardInspectionExample(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilDigitizer, MIL_ID MilGrabImage, I3DApi* p3DApi, config3DApi *pConfig)
   {
   MosPrintf(MIL_TEXT("[PARTICLE BOARD FLATNESS INSPECTION]\n\n")
             MIL_TEXT("In this example, a textured particle board is scanned to generate a depth map.\n")
             MIL_TEXT("The depth map is analyzed to detect depressions on the surface of the \n")
             MIL_TEXT("particle board.\n\n")
             MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   // Set the configuration parameters of Chromasens 3D API.

   MIL_ID MilDisparityImage;
   MIL_ID MilRectifiedImage;
   MIL_INT WorkSizeX;
   MIL_INT WorkSizeY;
   if(Initialize3DApi(p3DApi, pConfig, MilSystem, &MilGrabImage, 1, &MilDisparityImage, &MilRectifiedImage, &WorkSizeX, &WorkSizeY))
      {
      // Allocate the work images.
      MIL_ID MilCorrectedDepthMap  = MbufAlloc2d(MilSystem, WorkSizeX, WorkSizeY, 16+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID MilCorrectedWorkDepthMap  = MbufAlloc2d(MilSystem, WorkSizeX, WorkSizeY, 16+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID MilCorrectedWorkColorMap  = MbufAllocColor(MilSystem, 3, WorkSizeX, WorkSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID MilDefectImage        = MbufAlloc2d(MilSystem, WorkSizeX, WorkSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID MilPseudoColoredMap   = MbufAllocColor(MilSystem, 3, WorkSizeX, WorkSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);

      // Allocate the images for the 3D display.
      MIL_INT Display3DSizeX = (MIL_INT)(WorkSizeX * D3D_DISPLAY_SUBSAMPLING);
      MIL_INT Display3DSizeY = (MIL_INT)(WorkSizeY * D3D_DISPLAY_SUBSAMPLING);
      MIL_ID Mil3DDisplayDepthMap  = MbufAlloc2d(MilSystem, Display3DSizeX, Display3DSizeY, 16+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID Mil3DDisplayColorMap  = MbufAllocColor(MilSystem, 3, Display3DSizeX, Display3DSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
         
      // Allocate the jet color LUT.
      MIL_ID MilColorLut = MbufAllocColor(MilSystem, 3, MIL_UINT16_MAX, 1, 8+M_UNSIGNED, M_LUT, M_NULL);
         
      // Allocate the plane geometry.
      MIL_ID MilPlaneFit = M3dmapAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);

      // Allocate blob objects.
      MIL_ID MilBlobResult;
      MIL_ID MilBlobContext;

      MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);
      MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobResult);
      MblobControl(MilBlobContext, M_MIN_PIXEL, M_ENABLE);

      //Allocate graphics context
      MIL_ID BlobGraphicsContext;

      MgraAlloc(MilSystem, &BlobGraphicsContext);
      MgraColor(BlobGraphicsContext, 0);

      // Grab and calculate 3D.
      GrabImage(p3DApi, &MilDisplay, &MilDigitizer, &MilGrabImage, 1);
      Calculate3D(p3DApi, &MilDisplay, &MilGrabImage, 1, MilDisparityImage, MilRectifiedImage, MilCorrectedWorkDepthMap, MilCorrectedWorkColorMap);
         
      // Fill the holes of the depth map.
      FillHolesAndSmooth(MilDisplay, MilCorrectedWorkDepthMap, MilCorrectedDepthMap, PARTICLEBOARD_KERNEL_SIZE); 
         
      // Calibrate the depth map.
      MIL_DOUBLE ZRange = CalibrateDepthMap(MilCorrectedDepthMap, p3DApi, pConfig, 1, PARTICLEBOARD_Z_MULT_FACTOR);

      // Calculate a plane on the data. 
      M3dmapSetGeometry(MilPlaneFit, M_PLANE, M_FIT, (MIL_DOUBLE)MilCorrectedDepthMap, M_NULL, ZRange * PLANE_OUTLIER_DISTANCE_RANGE_FACTOR, M_DEFAULT, M_DEFAULT); 

      // Remove the plane from the depth map.
      M3dmapArith(MilCorrectedDepthMap, MilPlaneFit, MilCorrectedDepthMap, M_NULL, M_SUB, M_SET_WORLD_OFFSET_Z);
                       
      // Show the depth map corrected for the plane error.
      MosPrintf(MIL_TEXT("A plane, fitted on the data, was subtracted from the depth map.\n")
                  MIL_TEXT("The depth map now represents the heights relative to the fitted plane.\n\n")
                  MIL_TEXT("Press <Enter> to continue.\n\n"));
      ShowImage(MilDisplay, MilCorrectedDepthMap, true); 

      // Correct curve.
      CorrectHorizontalCurve(MilCorrectedDepthMap, HORIZONTAL_CURVE_CORRECTION_CHILD_OFFSET_Y, HORIZONTAL_CURVE_CORRECTION_CHILD_SIZE_Y);               
         
      // Show the depth map where the horizontal curve is corrected.
      MosPrintf(MIL_TEXT("Remaining horizontal lens distortion was corrected. The curve, obtained\n")
                  MIL_TEXT("using the average value of each column, was subtracted.\n\n")
                  MIL_TEXT("Press <Enter> to continue.\n\n"));
      ShowImage(MilDisplay, MilCorrectedDepthMap, true); 

      // Show the depth map in a 3d display.
      MIL_DISP_D3D_HANDLE DispHandle;
      MimResize(MilCorrectedDepthMap, Mil3DDisplayDepthMap, D3D_DISPLAY_SUBSAMPLING, D3D_DISPLAY_SUBSAMPLING, M_AVERAGE);
      MimResize(MilCorrectedWorkColorMap, Mil3DDisplayColorMap, D3D_DISPLAY_SUBSAMPLING, D3D_DISPLAY_SUBSAMPLING, M_AVERAGE);
      CalibrateDepthMap(Mil3DDisplayDepthMap, p3DApi, pConfig, 1.0 / D3D_DISPLAY_SUBSAMPLING, PARTICLEBOARD_Z_MULT_FACTOR);
      DispHandle = MdepthD3DAlloc(Mil3DDisplayDepthMap, Mil3DDisplayColorMap,
                                    D3D_DISPLAY_SIZE_X,
                                    D3D_DISPLAY_SIZE_Y,
                                    M_DEFAULT,
                                    M_DEFAULT,
                                    M_DEFAULT,
                                    M_DEFAULT,
                                    M_DEFAULT,
                                    M_DEFAULT,
                                    0);

      if (DispHandle != NULL)
         {
         MdispD3DShow(DispHandle);
         MdispD3DPrintHelp(DispHandle);
         }
         
      MosPrintf(MIL_TEXT("A 3D display of the particle board surface after rectification is shown.\n")
                  MIL_TEXT("For display purposes, the surface heights have been magnified by %.2f.\n\n")
                  MIL_TEXT("Press <Enter> to continue.\n\n"),
                  PARTICLEBOARD_Z_MULT_FACTOR);
      MosGetch();

      // Get the gray value at 0 height.
      MIL_DOUBLE FinalWorldPosZ;
      MIL_DOUBLE FinalGrayLevelSizeZ;
      McalInquire(MilCorrectedDepthMap, M_WORLD_POS_Z, &FinalWorldPosZ);
      McalInquire(MilCorrectedDepthMap, M_GRAY_LEVEL_SIZE_Z, &FinalGrayLevelSizeZ);
      MIL_DOUBLE DefectThresholdLowGray  = (DEFECT_THRESHOLD_LOW * PARTICLEBOARD_Z_MULT_FACTOR - FinalWorldPosZ) / FinalGrayLevelSizeZ;
      MIL_DOUBLE DefectThresholdHighGray = (DEFECT_THRESHOLD_HIGH * PARTICLEBOARD_Z_MULT_FACTOR- FinalWorldPosZ) / FinalGrayLevelSizeZ;

      // Get the possible defects.
      MimBinarize(MilCorrectedDepthMap, MilDefectImage, M_FIXED + M_LESS, DefectThresholdLowGray, M_NULL);
         
      // Perform seed recontruction using blob. 
      MblobCalculate(MilBlobContext, MilDefectImage, MilCorrectedDepthMap, MilBlobResult);
      MblobSelect(MilBlobResult, M_INCLUDE_ONLY, M_MIN_PIXEL, M_LESS, DefectThresholdHighGray, M_NULL);

      MIL_INT NbDefects;
      MblobGetResult(MilBlobResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbDefects);
      if(NbDefects>0)
         {
         MblobDraw(BlobGraphicsContext, MilBlobResult, MilDefectImage, M_DRAW_BLOBS, M_EXCLUDED_BLOBS, M_DEFAULT);
            
         // Color the defect blob with a color lut in the color map.
         MIL_ID MilColorLutChild = MbufChild1d(MilColorLut, (MIL_INT)DefectThresholdHighGray, (MIL_INT)DefectThresholdLowGray - (MIL_INT)DefectThresholdHighGray + 1, M_NULL);
         MbufClear(MilColorLut, M_RGB888(0,0,128));
         MgenLutFunction(MilColorLutChild, M_COLORMAP_JET, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         MimLutMap(MilCorrectedDepthMap, MilPseudoColoredMap, MilColorLut);
         MbufCopyCond(MilPseudoColoredMap, MilCorrectedWorkColorMap, MilDefectImage, M_EQUAL, 255);  
         MbufFree(MilColorLutChild);
         }
         
      // Show the defect in overlay of the displayed depth image.
      MIL_ID MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
      MbufCopyCond(MilPseudoColoredMap, MilOverlay, MilDefectImage, M_EQUAL, 255);

      // Show the defect in the 3D display.
      if (DispHandle != NULL)
         {
         MimResize(MilCorrectedWorkColorMap, Mil3DDisplayColorMap, D3D_DISPLAY_SUBSAMPLING, D3D_DISPLAY_SUBSAMPLING, M_AVERAGE);
         MdepthD3DSetImages(DispHandle, Mil3DDisplayDepthMap, Mil3DDisplayColorMap);
         }
      MosPrintf(MIL_TEXT("The defects were extracted using an hysteresis threshold defined in world\n")
                  MIL_TEXT("units. A first low height threshold(set to %.2f um) identifies groups of\n")
                  MIL_TEXT("pixels that could be defects. To be considered defects, the group of \n")
                  MIL_TEXT("pixels must have a least one height value greater than a second higher\n")
                  MIL_TEXT("threshold (set to %.2f um).\n\n")
                  MIL_TEXT("Press <Enter> to continue.\n\n"),
                  DEFECT_THRESHOLD_LOW * 1000,
                  DEFECT_THRESHOLD_HIGH* 1000);
      ShowImage(MilDisplay, MilCorrectedDepthMap, true);  

      // Clear the overlay and deselect the image.
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
         
      // Stop the calculation.
      p3DApi->stopBlocking();

      // Free the 3D display.
      if(DispHandle)
         MdispD3DFree(DispHandle);

      // Free the plane.
      M3dmapFree(MilPlaneFit);

      // Free blob.
      MblobFree(MilBlobContext);
      MblobFree(MilBlobResult);

      // Free the color lut.
      MbufFree(MilColorLut);

      // Free the work images.
      MbufFree(Mil3DDisplayColorMap);
      MbufFree(Mil3DDisplayDepthMap);
      MbufFree(MilPseudoColoredMap);
      MbufFree(MilDefectImage);
      MbufFree(MilCorrectedWorkColorMap);
      MbufFree(MilCorrectedWorkDepthMap);
      MbufFree(MilCorrectedDepthMap);

      //Free graphics context
      MgraFree(BlobGraphicsContext);
         
      // Free output images.
      if(MilRectifiedImage)
         MbufFree(MilRectifiedImage);
      MbufFree(MilDisparityImage);
      }
   }

//*****************************************************************************
// Sand paper inspection example parameters.
//*****************************************************************************
static const int           SAND_PAPER_3DAPI_DSTART       = -20;
static const int           SAND_PAPER_3DAPI_DEND         = 10;
static const int           SAND_PAPER_3DAPI_WINDOW_TYPE  = 0; // 27x27
static const double        SAND_PAPER_3DAPI_MIN_STD_DEV  = 0.5; 
static const int           SAND_PAPER_3DAPI_MIN_GRAY     = 10;
static const MIL_DOUBLE    SAND_PAPER_3DAPI_MIN_KKF      = 0.5;

static const MIL_INT    RESIZE_DOWN_NEIGHBORHOOD = 10;
static const MIL_DOUBLE RESIZE_DOWN_FACTOR       = 1.0/RESIZE_DOWN_NEIGHBORHOOD; 

static const MIL_INT LOCAL_DENSITY_KERNEL_SIZE = 45;

static const MIL_DOUBLE MIN_PEAK_HEIGHT = 0.25; // in mm

static const MIL_INT SAND_PAPER_KERNEL_SIZE = 51;

static const MIL_DOUBLE SAND_PAPER_Z_MULT_FACTOR = 4;
//*****************************************************************************
// SandPaperInspectionExample.  
//*****************************************************************************
void SandPaperInspectionExample(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilDigitizer, MIL_ID MilGrabImage, I3DApi* p3DApi, config3DApi *pConfig)
   {
   MosPrintf(MIL_TEXT("[SAND PAPER DENSITY INSPECTION]\n\n")
             MIL_TEXT("In this example, a piece of sand paper is scanned to generate a depth map.\n")
             MIL_TEXT("The depth map is analyzed to generate a local density map of the peaks.\n\n")
             MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   // Allocate the graphic list.
   MIL_ID MilGraList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   // Set the configuration parameters of Chromasens 3D API.
   pConfig->dStart = SAND_PAPER_3DAPI_DSTART;
   pConfig->dEnd   = SAND_PAPER_3DAPI_DEND;
   pConfig->windowType = SAND_PAPER_3DAPI_WINDOW_TYPE;
   pConfig->minStdDevA = SAND_PAPER_3DAPI_MIN_STD_DEV;
   pConfig->mingw = SAND_PAPER_3DAPI_MIN_GRAY;
   pConfig->minKkf = SAND_PAPER_3DAPI_MIN_KKF;

   MIL_ID MilDisparityImage;
   MIL_ID MilRectifiedImage;
   MIL_INT WorkSizeX;
   MIL_INT WorkSizeY;
   if(Initialize3DApi(p3DApi, pConfig, MilSystem, &MilGrabImage, 1, &MilDisparityImage, &MilRectifiedImage, &WorkSizeX, &WorkSizeY))
      {
      // Calculate the subsampled image size.
      MIL_INT SubsampledSizeX = (MIL_INT)(WorkSizeX * RESIZE_DOWN_FACTOR);
      MIL_INT SubsampledSizeY = (MIL_INT)(WorkSizeY * RESIZE_DOWN_FACTOR);

      // Allocate the work images.
      MIL_ID MilCorrectedDepthMap  = MbufAlloc2d(MilSystem, WorkSizeX, WorkSizeY, 16+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID MilCorrectedWorkDepthMap  = MbufAlloc2d(MilSystem, WorkSizeX, WorkSizeY, 16+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID MilCorrectedWorkColorMap  = MbufAllocColor(MilSystem, 3, WorkSizeX, WorkSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID MilSubsampledDepthMap = MbufAlloc2d(MilSystem, SubsampledSizeX, SubsampledSizeY, 16+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL); 
      MIL_ID MilPeakImage = MbufAlloc2d(MilSystem, SubsampledSizeX, SubsampledSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID MilZoneOfInfluenceImage = MbufAlloc2d(MilSystem, SubsampledSizeX, SubsampledSizeY, 16+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);

      // Allocate the images for the 3D display.
      MIL_INT Display3DSizeX = (MIL_INT)(WorkSizeX * D3D_DISPLAY_SUBSAMPLING);
      MIL_INT Display3DSizeY = (MIL_INT)(WorkSizeY * D3D_DISPLAY_SUBSAMPLING);
      MIL_ID Mil3DDisplayDepthMap  = MbufAlloc2d(MilSystem, Display3DSizeX, Display3DSizeY, 16+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MIL_ID Mil3DDisplayColorMap  = MbufAllocColor(MilSystem, 3, Display3DSizeX, Display3DSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);

      // Allocate the image to compute the local density.
      MIL_DOUBLE LocalPixelSize = pConfig->resolutionX / (RESIZE_DOWN_FACTOR);
      MIL_DOUBLE KernelSizeInMm = LocalPixelSize * LOCAL_DENSITY_KERNEL_SIZE;
      MIL_DOUBLE KernelSizeInMmSquare = KernelSizeInMm * KernelSizeInMm;
      MIL_DOUBLE KernelSizeInPixelSquare = LOCAL_DENSITY_KERNEL_SIZE * LOCAL_DENSITY_KERNEL_SIZE;
      MIL_ID MilLocalDensityImage = MbufAlloc2d(MilSystem, SubsampledSizeX, SubsampledSizeY, 8+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
      MIL_ID MilLocalDensityFullSizeImage = MbufAlloc2d(MilSystem, WorkSizeX, WorkSizeY, 8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, M_NULL);
         
      // Allocate the average kernel.
      MIL_ID MilAverageKernel = MbufAlloc2d(MilSystem, LOCAL_DENSITY_KERNEL_SIZE, LOCAL_DENSITY_KERNEL_SIZE, 8+M_UNSIGNED, M_KERNEL, M_NULL);
      MIL_INT KernelArea = GenAverageCircleKernel(MilAverageKernel);
      MIL_DOUBLE CircleKernelSizeInMmSquare = ((MIL_DOUBLE)KernelArea/KernelSizeInPixelSquare) * KernelSizeInMmSquare;
         
      // Allocate the peak list.
      MIL_INT MaxNbEvents = SubsampledSizeX * SubsampledSizeY / 9;
      MIL_ID MilPeakList = MimAllocResult(MilSystem, MaxNbEvents, M_EVENT_LIST, M_NULL);

      // Allocate the stat context.
      MIL_ID MilStatContext = MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_NULL);

      // Enable statistics to use.
      MimControl(MilStatContext, M_STAT_MAX, M_ENABLE);

      // Allocate the stat result.
      MIL_ID MilStatResult = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);

      // Allocate blob objects.
      MIL_ID MilBlobResult;
      MIL_ID MilBlobContext;

      MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);
      MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobResult);
      MblobControl(MilBlobContext, M_BLOB_IDENTIFICATION_MODE, M_LABELED_TOUCHING);
      MblobControl(MilBlobContext, M_MIN_PIXEL, M_ENABLE);
      MblobControl(MilBlobContext, M_MAX_PIXEL, M_ENABLE);

      // Grab and calculate 3D.
      GrabImage(p3DApi, &MilDisplay, &MilDigitizer, &MilGrabImage, 1);
      Calculate3D(p3DApi, &MilDisplay, &MilGrabImage, 1, MilDisparityImage, MilRectifiedImage, MilCorrectedWorkDepthMap, MilCorrectedWorkColorMap);         

      // Fill the holes of the depth map.
      FillHolesAndSmooth(MilDisplay, MilCorrectedWorkDepthMap, MilCorrectedDepthMap, SAND_PAPER_KERNEL_SIZE); 
         
      // Calibrate the depth map.
      CalibrateDepthMap(MilCorrectedDepthMap, p3DApi, pConfig, 1, SAND_PAPER_Z_MULT_FACTOR);
         
      // Subsample the depth map.
      MimResize(MilCorrectedDepthMap, MilSubsampledDepthMap, RESIZE_DOWN_FACTOR, RESIZE_DOWN_FACTOR, M_AVERAGE);

      // Locate the possible peaks.
      MimLocateEvent(MilSubsampledDepthMap, MilPeakList,  M_ALL+M_LOCAL_MAX_STRICT_MEDIUM, M_NULL, M_NULL);
      MIL_INT NbEvent;
      MimGetResult(MilPeakList, M_NB_EVENT + M_TYPE_MIL_INT, &NbEvent);
      MIL_INT* pCoordX = new MIL_INT[MaxNbEvents];
      MIL_INT* pCoordY = new MIL_INT[MaxNbEvents];
      MimGetResult(MilPeakList, M_POSITION_X + M_TYPE_MIL_INT, pCoordX);
      MimGetResult(MilPeakList, M_POSITION_Y + M_TYPE_MIL_INT, pCoordY);

      // Create a peak image an get their zone of influence.
      MgraColor(M_DEFAULT, 255);
      MbufClear(MilPeakImage, 0);
      MgraDots(M_DEFAULT, MilPeakImage, NbEvent, pCoordX, pCoordY, M_DEFAULT);
      MimZoneOfInfluence(MilPeakImage, MilZoneOfInfluenceImage, M_CHAMFER_3_4);

      // Filter the peaks based on their contrast.
      MIL_INT* pValidCoordX = new MIL_INT[NbEvent];
      MIL_INT* pValidCoordY = new MIL_INT[NbEvent];
      MIL_INT NbBlobs = 0;
      MIL_INT NbValidPeak = 0;

      MblobCalculate(MilBlobContext, MilZoneOfInfluenceImage, MilSubsampledDepthMap, MilBlobResult);
      MblobGetResult(MilBlobResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbBlobs);

      // NbBlobs should be equal to NbEvents.
      if(NbBlobs == NbEvent)
         {
         // Get the minimum value in the zone of influence.
         MIL_INT* pMinValue = new MIL_INT[NbBlobs];
         MblobGetResult(MilBlobResult, M_DEFAULT, M_MIN_PIXEL + M_TYPE_MIL_INT, pMinValue);
            
         // Get the data pointer and the pitch of the subsampled and zone of influence image to access values directly.
         MIL_UINT16* pZoneOfInfluenceData = (MIL_UINT16*)MbufInquire(MilZoneOfInfluenceImage, M_HOST_ADDRESS, M_NULL);
         MIL_INT ZonePitch = MbufInquire(MilZoneOfInfluenceImage, M_PITCH, M_NULL);
         MIL_UINT16* pSubsampledImageData = (MIL_UINT16*)MbufInquire(MilSubsampledDepthMap, M_HOST_ADDRESS, M_NULL);
         MIL_INT SubsampledPitch = MbufInquire(MilSubsampledDepthMap, M_PITCH, M_NULL);
            
         for(MIL_INT PeakIdx = 0; PeakIdx < NbEvent; PeakIdx++)
            {
            // Get the gray value at the peak.
            MIL_INT PeakValue = pSubsampledImageData[pCoordX[PeakIdx] + SubsampledPitch*pCoordY[PeakIdx]];
               
            // Get the minimum value in the zone of influence associated to the peak.
            MIL_INT PeakLabel = pZoneOfInfluenceData[pCoordX[PeakIdx] + ZonePitch*pCoordY[PeakIdx]];

            // Calculate the height associated to the gray value contrast.
            MIL_INT PeakContrast =  PeakValue - pMinValue[PeakLabel-1];
            float MinZ;
            p3DApi->grayToMm(MinZ, (unsigned short)1);
            float PeakHeight;
            p3DApi->grayToMm(PeakHeight, (unsigned short)PeakContrast);
            PeakHeight = MinZ - PeakHeight;
               
            // If the peak height is above the threshold, keep the peak.
            if(PeakHeight >= MIN_PEAK_HEIGHT)
               {
               pValidCoordX[NbValidPeak] = pCoordX[PeakIdx];
               pValidCoordY[NbValidPeak] = pCoordY[PeakIdx];
               NbValidPeak++;
               }
            }
         delete [] pMinValue;
             
         // Draw the valid peaks over the original image.
         MgraColor(M_DEFAULT, M_COLOR_GREEN);   
         for(MIL_INT PeakIdx = 0; PeakIdx < NbValidPeak; PeakIdx++)
            MgraArcFill(M_DEFAULT, MilGraList, pValidCoordX[PeakIdx], pValidCoordY[PeakIdx], 0.5, 0.5, 0, 360);
         MgraControlList(MilGraList, M_ALL, M_DEFAULT, M_DRAW_ZOOM_X, RESIZE_DOWN_NEIGHBORHOOD);
         MgraControlList(MilGraList, M_ALL, M_DEFAULT, M_DRAW_ZOOM_Y, RESIZE_DOWN_NEIGHBORHOOD);
            
         // Draw the peaks in the rectified color image.
         MgraDraw(MilGraList, MilCorrectedWorkColorMap, M_DEFAULT);
         
         // Associate the graphic list to the display.
         MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

         // Show the depth map in a 3D display.
         MIL_DISP_D3D_HANDLE DispHandle;
         MimResize(MilCorrectedDepthMap, Mil3DDisplayDepthMap, D3D_DISPLAY_SUBSAMPLING, D3D_DISPLAY_SUBSAMPLING, M_AVERAGE);
         MimResize(MilCorrectedWorkColorMap, Mil3DDisplayColorMap, D3D_DISPLAY_SUBSAMPLING, D3D_DISPLAY_SUBSAMPLING, M_AVERAGE);
         CalibrateDepthMap(Mil3DDisplayDepthMap, p3DApi, pConfig, 1.0 / D3D_DISPLAY_SUBSAMPLING, SAND_PAPER_Z_MULT_FACTOR);
         DispHandle = MdepthD3DAlloc(Mil3DDisplayDepthMap, Mil3DDisplayColorMap,
                                       D3D_DISPLAY_SIZE_X,
                                       D3D_DISPLAY_SIZE_Y,
                                       M_DEFAULT,
                                       M_DEFAULT,
                                       M_DEFAULT,
                                       M_DEFAULT,
                                       M_DEFAULT,
                                       M_DEFAULT,
                                       0);

         if (DispHandle != NULL)
            {
            MdispD3DShow(DispHandle);
            MdispD3DPrintHelp(DispHandle);
            }
            
         MosPrintf(MIL_TEXT("A 3D display of the sand paper is shown.\n")
                     MIL_TEXT("For display purposes, the surface heights have been magnified by %.2f.\n")
                     MIL_TEXT("The peaks have been detected and are identified in green.\n\n")
                     MIL_TEXT("Press <Enter> to continue.\n\n"),
                     SAND_PAPER_Z_MULT_FACTOR);
         ShowImage(MilDisplay, MilCorrectedDepthMap, true); 
            
         // Calculate the global peak density in peak/cm^2.
         MIL_DOUBLE GlobalPeakDensity = 100 * (MIL_DOUBLE)NbValidPeak / (WorkSizeX * WorkSizeY* pConfig->resolutionX * pConfig->resolutionX);

         // Generate an image indicating the local peak density per KernelSizeInMmSquare. 
         MbufClear(MilPeakImage, 0);
         MgraColor(M_DEFAULT, 1);
         MgraDots(M_DEFAULT, MilPeakImage, NbValidPeak, pValidCoordX, pValidCoordY, M_DEFAULT);
         MimConvolve(MilPeakImage, MilLocalDensityImage, MilAverageKernel);
            
         // Put the density per cm^2.
         MimArith(MilLocalDensityImage, 100.0/CircleKernelSizeInMmSquare, MilLocalDensityImage, M_MULT_CONST+M_FLOAT_PROC+M_SATURATION);

         // Increase contrast.
         MIL_DOUBLE MaxDensity;
         MimStatCalculate(MilStatContext, MilLocalDensityImage, MilStatResult, M_DEFAULT);

         MimGetResult(MilStatResult, M_STAT_MAX, &MaxDensity);
         MimArith(MilLocalDensityImage, 255.0/MaxDensity, MilLocalDensityImage, M_MULT_CONST + M_SATURATION + M_FLOAT_PROC);

         // Resize to fit in the full image.
         MimResize(MilLocalDensityImage, MilLocalDensityFullSizeImage, (MIL_DOUBLE)RESIZE_DOWN_NEIGHBORHOOD, (MIL_DOUBLE)RESIZE_DOWN_NEIGHBORHOOD, M_INTERPOLATE);
            
         // Show the peak density. 
         if (DispHandle != NULL)
            {
            MimResize(MilLocalDensityFullSizeImage, Mil3DDisplayColorMap, D3D_DISPLAY_SUBSAMPLING, D3D_DISPLAY_SUBSAMPLING, M_AVERAGE);
            MdepthD3DSetImages(DispHandle, Mil3DDisplayDepthMap, Mil3DDisplayColorMap);
            }
         MosPrintf(MIL_TEXT("The 3D display now shows the local density of peaks.\n")
                     MIL_TEXT("The global average density is %.2f peaks/cm^2.\n")
                     MIL_TEXT("The maximum local density (in white) is %.2f peak/cm^2.\n\n")
                     MIL_TEXT("Press <Enter> to continue.\n\n"),
                     GlobalPeakDensity,
                     MaxDensity);
         ShowImage(MilDisplay, MilLocalDensityFullSizeImage, false);

         // Free the 3D display.
         if(DispHandle)
            MdispD3DFree(DispHandle);
         }
      else
         MosPrintf(MIL_TEXT("The number of possible peaks should be equal to nb of zone of influence.\n\n"));
         
      // Free the allocated arrays.
      delete [] pValidCoordY;
      delete [] pValidCoordX;
      delete [] pCoordY;
      delete [] pCoordX;

      // Stop the calculation.
      p3DApi->stopBlocking();

      // Free blob.
      MblobFree(MilBlobContext);
      MblobFree(MilBlobResult);

      // Free the average kernel.
      MbufFree(MilAverageKernel);
         
      // Free the stat list
      MimFree(MilStatResult);

      // Free the stat context.
      MimFree(MilStatContext);

      // Free the peak list.
      MimFree(MilPeakList);

      // Free the work images.
      MbufFree(Mil3DDisplayColorMap);
      MbufFree(Mil3DDisplayDepthMap);
      MbufFree(MilLocalDensityFullSizeImage);
      MbufFree(MilLocalDensityImage);
      MbufFree(MilZoneOfInfluenceImage);
      MbufFree(MilPeakImage);
      MbufFree(MilSubsampledDepthMap);
      MbufFree(MilCorrectedWorkColorMap);
      MbufFree(MilCorrectedWorkDepthMap);
      MbufFree(MilCorrectedDepthMap);
         
      // Free output images.
      if(MilRectifiedImage)
         MbufFree(MilRectifiedImage);
      MbufFree(MilDisparityImage);
      }

   // Free the graphic list
   MgraFree(MilGraList);
   }

//*****************************************************************************
// GrabImage. Grabs the image from the 3DPIXA.
//*****************************************************************************
void GrabImage(I3DApi* p3DApi, MIL_ID* pMilDisplays, MIL_ID* pMilDigitizers, MIL_ID* pMilGrabImages, MIL_INT NbCamera)
   {
   MIL_CONST_TEXT_PTR ImageText = NbCamera == 1 ? MIL_TEXT("An image") : MIL_TEXT("Two images");
   if(SYSTEM_TO_USE == 0)
      {
      MosPrintf(MIL_TEXT("%s, grabbed with the the 3DPIXA camera, will be loaded.\n\n")
                MIL_TEXT("Press <Enter> to grab.\n\n"),
                ImageText);
      }
   else
      {
      MosPrintf(MIL_TEXT("%s will be grabbed with the the 3DPIXA camera.\n\n")
                MIL_TEXT("Press <Enter> to grab.\n\n"),
                ImageText);
      }   
   MosGetch();

   // Start thread to generate movement of the object and send the trigger to the frame grabber.
   MIL_ID MilSystem = MdigInquire(pMilDigitizers[0], M_OWNER_SYSTEM, M_NULL);
   MIL_ID MilStartScanThread = MthrAlloc(MilSystem, M_THREAD, M_DEFAULT, StartScan, M_NULL, M_NULL);

   // Set the second digitizer to be asynchronous and start the grab.
   if(NbCamera == 2)
      {
      if(SYSTEM_TO_USE != 0)
         MdigControl(pMilDigitizers[1], M_GRAB_MODE, M_ASYNCHRONOUS);
      MdigGrab(pMilDigitizers[1], pMilGrabImages[1]);
      }
   MdigGrab(pMilDigitizers[0], pMilGrabImages[0]);

   // Free the thread.
   MthrFree(MilStartScanThread);

   // Show the grabbed image.
   MosPrintf(MIL_TEXT("The displayed image was grabbed with the 3DPIXA camera.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   ShowImage(pMilDisplays[0], pMilGrabImages[0], false);
   if(NbCamera == 2)
      {
      MosPrintf(MIL_TEXT("The displayed image was grabbed with the 3DPIXA camera.\n\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      ShowImage(pMilDisplays[1], pMilGrabImages[1], false);
      }
   }

//*****************************************************************************
// Calculate3D. Calculates the 3D data with the CS3D API.
//*****************************************************************************
void Calculate3D(I3DApi* p3DApi, MIL_ID* pMilDisplays, MIL_ID* pMilSrcImages, MIL_INT NbSrcImage, MIL_ID MilDisparityImage, MIL_ID MilRectifiedImage, MIL_ID MilCorrectedWorkDepthMap, MIL_ID MilCorrectedWorkColorMap)
      {
   // Load the source images in the 3D API.
   for(int SrcIdx = 0; SrcIdx < NbSrcImage; SrcIdx++)
      {
      char* pImageData = (char*)MbufInquire(pMilSrcImages[SrcIdx], M_HOST_ADDRESS, M_NULL);
      MosPrintf(MIL_TEXT("CS3D: Loading the image from grab %i in the CS3D API..."), SrcIdx);
      if(p3DApi->setSrcImgPtr(SrcIdx, pImageData) < 0)
         MosPrintf(MIL_TEXT("Image info not acceptable for calculation.\n"));
      else
         MosPrintf(MIL_TEXT("Done.\n"));
      p3DApi->setSrcImgLoaded(SrcIdx);
      }
   MosPrintf(MIL_TEXT("\n"));

   // Get the resulting image.
   MosPrintf(MIL_TEXT("CS3D: Calculating the depth map..."));
   p3DApi->getNextImgBlocking();

   void* pDisparityData = (void*)MbufInquire(MilDisparityImage, M_HOST_ADDRESS, M_NULL);
   int DisparityPitchByte = (int)MbufInquire(MilDisparityImage, M_PITCH_BYTE, M_NULL);
   p3DApi->getLastImage(&pDisparityData, DisparityPitchByte, IMG_OUT_DISP);
   
   if(MilRectifiedImage)
      {
      void* pRectifiedData = (void*)MbufInquire(MilRectifiedImage, M_HOST_ADDRESS, M_NULL);
      int RectifiedPitchByte = (int)MbufInquire(MilRectifiedImage, M_PITCH_BYTE, M_NULL);
      MIL_INT SizeBand = MbufInquire(MilRectifiedImage, M_SIZE_BAND, M_NULL);
      p3DApi->getLastImage(&pRectifiedData, (int)RectifiedPitchByte, SizeBand == 1 ? IMG_OUT_GRAY : IMG_OUT_BGRA);
      }
   MosPrintf(MIL_TEXT("Done\n\n"));

   // Get only the workable area of the disparity map.
   MIL_INT WorkSizeX = MbufInquire(MilCorrectedWorkDepthMap, M_SIZE_X, M_NULL);
   MIL_INT WorkSizeY = MbufInquire(MilCorrectedWorkDepthMap, M_SIZE_Y, M_NULL);
   MbufCopyColor2d(MilDisparityImage, MilCorrectedWorkDepthMap, 0, BORDER_SIZE_X, 0, 0, 0, 0, WorkSizeX, WorkSizeY);
   MIL_ID MilSourceRectifiedImage = MilRectifiedImage == 0 ? MilDisparityImage : MilRectifiedImage;
   MbufCopyColor2d(MilSourceRectifiedImage, MilCorrectedWorkColorMap, M_ALL_BANDS, BORDER_SIZE_X, 0, M_ALL_BANDS, 0, 0, WorkSizeX, WorkSizeY);

   // Show the disparity image.
   MosPrintf(MIL_TEXT("The depth map is displayed.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   if(NbSrcImage == 2)
      MdispSelect(pMilDisplays[1], M_NULL);
   ShowImage(pMilDisplays[0], MilCorrectedWorkDepthMap, true);
   }


//*****************************************************************************
// FillHolesAndSmooth. Smooths the depth map and fills its hole. Invalid pixels
//                     whose neighborhood contains at least 10% of valid pixels
//                     are replaced by the average of the valid neighbors.
//*****************************************************************************
void FillHolesAndSmooth(MIL_ID MilDisplay, MIL_ID MilDepthMap, MIL_ID MilFilledHolesDepthMap, MIL_INT FilterSize)
   {
   MIL_ID MilSystem = MbufInquire(MilDepthMap, M_OWNER_SYSTEM, M_NULL);
   MIL_INT ImageSizeX = MbufInquire(MilDepthMap, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(MilDepthMap, M_SIZE_Y, M_NULL);

   // Allocate the work images.
   MIL_ID MilValidImage = MbufAlloc2d(MilSystem, ImageSizeX, ImageSizeY, 16+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MIL_ID MilNormImage = MbufAlloc2d(MilSystem, ImageSizeX, ImageSizeY, 16+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MIL_ID MilTempFilledHolesDepthMap32 = MbufAlloc2d(MilSystem, ImageSizeX, ImageSizeY, 32 + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);

   // Create the valid image.
   MimBinarize(MilDepthMap, MilValidImage, M_FIXED + M_GREATER, 0, M_NULL);

   MIL_ID MilUniformKernel = MbufAlloc2d(MilSystem, FilterSize, FilterSize, 8+M_UNSIGNED, M_KERNEL, M_NULL);
   MbufClear(MilUniformKernel, 1);
   MbufControl(MilUniformKernel, M_NORMALIZATION_FACTOR, FilterSize*FilterSize);
   MimConvolve(MilValidImage, MilNormImage, MilUniformKernel);
   MimConvolve(MilDepthMap, MilFilledHolesDepthMap, MilUniformKernel);
   
   // Normalize the values.
   MimShift(MilFilledHolesDepthMap, MilTempFilledHolesDepthMap32, 16);
   MimArith(MilTempFilledHolesDepthMap32, MilNormImage, MilFilledHolesDepthMap, M_DIV);

   // Invalidate the data of values that do not have enough valid values in their neighborhood.
   MimBinarize(MilNormImage, MilValidImage, M_GREATER, 0.1 * MIL_UINT16_MAX, M_NULL);
   MbufClearCond(MilFilledHolesDepthMap, MIL_UINT16_MAX, M_NULL, M_NULL, MilValidImage, M_EQUAL, 0);

   // Show the depth map without the holes.
   MosPrintf(MIL_TEXT("The depth map was smoothed and its holes were filled.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   ShowImage(MilDisplay, MilFilledHolesDepthMap, true);

   MbufFree(MilUniformKernel);
   MbufFree(MilTempFilledHolesDepthMap32);
   MbufFree(MilNormImage);
   MbufFree(MilValidImage);
   }

//*****************************************************************************
// CorrectHorizontalCurve. Corrects the horizontal curve of the depth map, 
//                         assuming that it should be flat.
//*****************************************************************************
void CorrectHorizontalCurve(MIL_ID MilDepthMap, MIL_INT ChildOffsetY, MIL_INT ChildSizeY)
   {
   MIL_ID MilSystem = MbufInquire(MilDepthMap, M_OWNER_SYSTEM, M_NULL);
   MIL_INT ImageSizeX = MbufInquire(MilDepthMap, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(MilDepthMap, M_SIZE_Y, M_NULL);
   
   // Allocate the temporary images and get the child.
   MIL_ID MilCorrectionSourceChild = MbufChild2d(MilDepthMap, 0, ChildOffsetY, ImageSizeX, ChildSizeY, M_NULL);
   MIL_ID MilAverageColumn = MbufAlloc2d(MilSystem, ImageSizeX, 1, 16+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MIL_ID MilAverageImage = MbufAlloc2d(MilSystem, ImageSizeX, ImageSizeY, 16+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   
   // Get the average of each column to get an estimation of the horizontal curve.
   MimResize(MilCorrectionSourceChild, MilAverageColumn, M_FILL_DESTINATION, M_FILL_DESTINATION, M_AVERAGE);
   MimResize(MilAverageColumn, MilAverageImage, M_FILL_DESTINATION, M_FILL_DESTINATION, M_NEAREST_NEIGHBOR);
   
   // Remove the curve.
   McalAssociate(MilDepthMap, MilAverageImage, M_DEFAULT);
   M3dmapArith(MilDepthMap, MilAverageImage, MilDepthMap, M_NULL, M_SUB, M_SET_WORLD_OFFSET_Z);

   // Free the allocations.
   MbufFree(MilAverageImage);
   MbufFree(MilAverageColumn);
   MbufFree(MilCorrectionSourceChild);
   }

//*****************************************************************************
// CalibrateDepthMap. Calibrates the depth map based on the configuration of the
//                    3DPIXA. Returns the Z-range.
//*****************************************************************************
MIL_DOUBLE CalibrateDepthMap(MIL_ID MilDepthMap, I3DApi* p3DApi, config3DApi *pConfig, MIL_DOUBLE XYMultFactor, MIL_DOUBLE ZMultFactor)
   {
   MIL_DOUBLE PixelSize = pConfig->resolutionX * XYMultFactor;
   McalUniform(MilDepthMap, 0, 0, PixelSize, PixelSize, 0.0, M_DEFAULT);
   float MinZ;
   p3DApi->grayToMm(MinZ, (unsigned short)65535);
   float MaxZ;
   p3DApi->grayToMm(MaxZ, (unsigned short)1);
   MinZ *= (float)ZMultFactor;
   MaxZ *= (float)ZMultFactor;
   McalControl(MilDepthMap, M_WORLD_POS_Z, MaxZ);
   McalControl(MilDepthMap, M_GRAY_LEVEL_SIZE_Z, (MinZ - MaxZ) / 65535);
   return MaxZ - MinZ;
   }

//*****************************************************************************
// GenAverageCircleKernel. Generates an average circle kernel and returns the
//                         circle's area.
//*****************************************************************************
MIL_INT GenAverageCircleKernel(MIL_ID MilAverageKernel)
   {
   // Get the kernel information.
   MIL_ID MilSystem = MbufInquire(MilAverageKernel, M_OWNER_SYSTEM, M_NULL);
   MIL_ID KernelSize = MbufInquire(MilAverageKernel, M_SIZE_X, M_NULL);
   MIL_INT KernelPitch = MbufInquire(MilAverageKernel, M_PITCH, M_NULL);

   // Allocate the stat context.
   MIL_ID MilStatContext = MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_NULL);

   // Enable statistics to use.
   MimControl(MilStatContext, M_STAT_NUMBER,    M_ENABLE);
   MimControl(MilStatContext, M_CONDITION, M_EQUAL);
   MimControl(MilStatContext, M_COND_LOW,  1);

   // Allocate a stat result.
   MIL_ID MilStatResult = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);

   // Create an image on top of the kernel.
   MIL_ID MilAverageKernelImage = MbufCreate2d(MilSystem, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_IMAGE+M_PROC, M_MIL_ID+M_PITCH, KernelPitch, (void*)MilAverageKernel, M_NULL); 

   // Clear the kernel.
   MbufClear(MilAverageKernel, 0);
     
   // Draw a circle in the middle of the kernel.
   MgraColor(M_DEFAULT, 1);
   MIL_INT CircleRadius = (KernelSize-1)/2;
   MgraArcFill(M_DEFAULT, MilAverageKernelImage, CircleRadius, CircleRadius, CircleRadius, CircleRadius, 0, 360);
   
   // Get the area of the circle.
   MimStatCalculate(MilStatContext, MilAverageKernelImage, MilStatResult, M_DEFAULT);

   MIL_INT CircleArea;
   MimGetResult(MilStatResult, M_STAT_NUMBER + M_TYPE_MIL_INT, &CircleArea);

   // Free the image created on the kernel.
   MbufFree(MilAverageKernelImage);

   // Free the stat result.
   MimFree(MilStatResult);

   // Free the stat context.
   MimFree(MilStatContext);

   return CircleArea;
   }

//*****************************************************************************
// ShowImage. Shows an image and waits for the user to press a key.
//*****************************************************************************
void ShowImage(MIL_ID MilDisplay, MIL_ID MilImage, bool AutoScale)
   {
   // Set the scale mode.
   MdispControl(MilDisplay, M_VIEW_MODE, AutoScale ? M_AUTO_SCALE : M_DEFAULT);

   // Select the image.
   MdispSelect(MilDisplay, MilImage);

   // Enable the display updates.
   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
   
   // Wait for the user to press a key.
   MosGetch();

   // Disable the display updates.
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   }

//*****************************************************************************
// StartScan Thread used to start the scanning of the object.
//*****************************************************************************
MIL_UINT32 MFTYPE StartScan(void *UserDataPtr)
   {
   // Open the communication with the scanner.

   // Send a command to start the scanner script.
   // By using a scanner with a controller, the command 
   // starts a program that initiates the movement
   // and sends the trigger to the frame grabber to start
   // grabbing.

   // Close the communication with the scanner.

   return 0;
   }

//*****************************************************************************
// AccessDll. Create the 3DAPI object.
//*****************************************************************************
typedef int (*newFunc)(void*, void*);
bool AccessDll(HINSTANCE* phDll, I3DApi** pp3DApi, config3DApi **ppConfig, MIL_CONST_TEXT_PTR DllName, char* FactoryProcName, char* ConfigFile)
   {
#if USE_CS3D_API

   MIL_TEXT_CHAR DllPath[MAX_PATH];
   MosSprintf(DllPath, MAX_PATH, MIL_TEXT("%s\\%s"), CS3D_DLL_PATH, DllName);

   // Load the DLL.
   *phDll = LoadLibrary(DllPath);
   if(*phDll == 0)
      {
      DWORD LastError = GetLastError();
      MosPrintf(MIL_TEXT("Error loading dll %s\n"), DllName);
      MosPrintf(MIL_TEXT("Error code: %d\n"), LastError);
      *pp3DApi = 0;
      return false;
      }

   // Get address of the factory function.
   newFunc CreateFuncAddr  = (newFunc)GetProcAddress(*phDll, FactoryProcName);

   if(CreateFuncAddr == 0)
      {
      MosPrintf(MIL_TEXT("Error: Cannot find function %s\n"), FactoryProcName);
      *phDll = 0;
      *pp3DApi = 0;
      return false;
      }

   // Use create-function with parameter.
   if(CreateFuncAddr(pp3DApi, ConfigFile) < 0)
      {
      MosPrintf(MIL_TEXT("Error: unable to create dll object\n"));
      *phDll = 0;
      *pp3DApi = 0;
      return false;
      }

   // Get the 3DPIXA configuration.
   *ppConfig = (*pp3DApi)->getConfig();
   if(!*ppConfig)
      {
      MosPrintf(MIL_TEXT("Error: unable to get 3d API configuration.\n"));
      return false;
      }

   return true;

# else

   // Allocate the stub I3DApi and get the pointer to the config.
   *pp3DApi = new I3DApi(COMPACT_STANDALONE_OUTPUT_IMAGE_PATH);
   *ppConfig = (*pp3DApi)->getConfig();
   return true;

#endif
   }

//*****************************************************************************
// Initialize3DApi. Initialize the 3DAPI object.
//*****************************************************************************
bool Initialize3DApi(I3DApi* p3DApi,
                     config3DApi *pConfig,
                     MIL_ID MilSystem,
                     MIL_ID* pMilSrcImages,
                     MIL_INT NbSrcImages,
                     MIL_ID* pMilDisparityImage,
                     MIL_ID* pMilRectifiedImage,
                     MIL_INT* pWorkSizeX,
                     MIL_INT* pWorkSizeY)
   {
   // Initialize the API.
   if(p3DApi->initialize(pConfig) >= 0)
      {
      // Set the source image information.
      for(int SrcIdx = 0; SrcIdx < NbSrcImages; SrcIdx++)
         {
         int ImageSizeX = (int)MbufInquire(pMilSrcImages[SrcIdx], M_SIZE_X, M_NULL);
         int ImageSizeY = (int)MbufInquire(pMilSrcImages[SrcIdx], M_SIZE_Y, M_NULL);
         char* pImageData = (char*)MbufInquire(pMilSrcImages[SrcIdx], M_HOST_ADDRESS, M_NULL);
         int ImagePitchByte = (int)MbufInquire(pMilSrcImages[SrcIdx], M_PITCH_BYTE, M_NULL);
         MIL_INT NbBands = MbufInquire(pMilSrcImages[SrcIdx], M_SIZE_BAND, M_NULL);
         if(NbBands == 3)
            {
            p3DApi->setSrcImgChannelOrder(SrcIdx, CO_BGRA);
            p3DApi->setSrcImgInfo(SrcIdx, ImageSizeX, ImageSizeY, 3, 32, ImagePitchByte, ImageSizeX*ImageSizeY*4);
            }
         else
            {
            p3DApi->setSrcImgChannelOrder(SrcIdx, CO_GRAY);
            p3DApi->setSrcImgInfo(SrcIdx, ImageSizeX, ImageSizeY, 1, 8, ImagePitchByte, ImageSizeX*ImageSizeY);
            }
         if(p3DApi->setSrcImgPtr(SrcIdx, pImageData) < 0)
            {
            MosPrintf(MIL_TEXT("Image %i info not acceptable for calculation.\n"), SrcIdx);
            return false;
            }
         }

      // Allocate MIL disparity output images.
      int DestSizeX;
      int DestSizeY;
      int DestChannel;
      unsigned long long DestSizeByte;
      p3DApi->getDestImgInfo(IMG_OUT_DISP, DestSizeX, DestSizeY, DestChannel, DestSizeByte);
      if(DestSizeX != -1)
         MbufAlloc2d(MilSystem, DestSizeX, DestSizeY, 16+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, pMilDisparityImage);
      else
         {
         MosPrintf(MIL_TEXT("Unable to get the output disparity image.\n"));
         return false;
         }
      
      // Allocate the color or gray image.
      MIL_INT ColorAttribute = M_BGR32;
      outImgType DestImageType = IMG_OUT_BGRA;
      if(pConfig->numChannelsUsedForCalculation == 1)
         {
         ColorAttribute = M_NULL;
         DestImageType = IMG_OUT_GRAY;
         }
      p3DApi->getDestImgInfo(DestImageType, DestSizeX, DestSizeY, DestChannel, DestSizeByte);
      if(DestSizeX != -1)
         MbufAllocColor(MilSystem, pConfig->numChannelsUsedForCalculation, DestSizeX, DestSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC + M_DISP + ColorAttribute, pMilRectifiedImage);
      else
         MosPrintf(MIL_TEXT("Unable to get the output color or grayscale rectified image.\n"));
      
      // Calculate the work size.
      *pWorkSizeX = DestSizeX - 2 * BORDER_SIZE_X;
      *pWorkSizeY = DestSizeY;

      // Start the calculation.
      MosPrintf(MIL_TEXT("CS3D: Starting the CS3D api..."));
      if(p3DApi->start() < 0)
         {
         MosPrintf(MIL_TEXT("Unable to start CS3D api.\n"));
         return false;
         }
      else
         {
         MosPrintf(MIL_TEXT("Done.\n\n"));
         }
      }
   else
      {
      MosPrintf(MIL_TEXT("Unable to initialize the CS3D api."));
      return false;
      }

   return true;
   }

//*****************************************************************************
// FreeDll. Free the 3DAPI object.
//*****************************************************************************
void FreeDll(HINSTANCE* phDll, I3DApi** pp3DApi)
   {
   if(*pp3DApi)
      {
      delete *pp3DApi;
      *pp3DApi = 0;
      }
#if USE_CS3D_API
   if(*phDll)
      {
      FreeLibrary(*phDll);
      *phDll = 0;
      }
#endif
   }

//*******************************************************************************
// CheckForRequiredMILFile. Checks that required avi is present to run the example.
//                          Generates an error if it's not present.
//*******************************************************************************
bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR  FileName)
   {
   MIL_INT FilePresent;
   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }
   return FilePresent == M_YES;
   }
