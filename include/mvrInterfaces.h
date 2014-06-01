// ***************************************************************
//  mvrInterfaces.h           version: 1.0.7  �  date: 2014-01-18
//  -------------------------------------------------------------
//  various interfaces exported by madVR
//  -------------------------------------------------------------
//  Copyright (C) 2011 - 2014 www.madshi.net, BSD license
// ***************************************************************

// 2014-01-18 1.0.7 added IMadVRSettings2
// 2013-06-04 1.0.6 added IMadVRInfo
// 2013-01-23 1.0.5 added IMadVRSubclassReplacement
// 2012-11-18 1.0.4 added IMadVRExternalPixelShaders
// 2012-10-07 1.0.3 added IMadVRExclusiveModeCallback
// 2011-08-03 1.0.2 added IMadVRExclusiveModeControl
// 2011-07-17 1.0.1 added IMadVRRefreshRateInfo
// 2011-06-25 1.0.0 initial release

#ifndef __mvrInterfaces__
#define __mvrInterfaces__

// ---------------------------------------------------------------------------
// IMadVR
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// IMadVROsdServices
// ---------------------------------------------------------------------------

// this interface allows you to draw your own graphical OSD
// your OSD will work in both windowed + exclusive mode
// there are 2 different ways to draw your OSD:

// (1) using bitmaps
// you can create multiple OSD elements
// each OSD element gets a name, a bitmap and a position
// the bitmap must be 24bit or 32bit RGB(A)
// for transparency you can use a color key or an 8bit alpha channel

// (2) using render callbacks
// you can provide madVR with callbacks
// these callbacks are then called during rendering
// one callback will be called the rendering target was cleared
// another callback will be called after rendering was fully completed
// in your callbacks you can modify the render target any way you like

// ---------------------------------------------------------------------------

// when using the (1) bitmaps method, you can register a mouse callback
// this callback will be called whenever a mouse event occurs
// mouse pos (0, 0) is the left top corner of the OSD bitmap element
// return "true" if your callback has handled the mouse message and
// if you want the mouse message to be "eaten" (instead of passed on)
typedef void (__stdcall *OSDMOUSECALLBACK)(LPCSTR name, LPVOID context, UINT message, WPARAM wParam, int posX, int posY);

// when using the (2) render callbacks method, you need to provide
// madVR with an instance of the IOsdRenderCallback interface
// it contains three callbacks you have to provide
[uuid("57FBF6DC-3E5F-4641-935A-CB62F00C9958")]
interface IOsdRenderCallback : public IUnknown
{
  // "SetDevice" is called when you register the callbacks
  // it provides you with the D3D device object used by madVR
  // when SetDevice is called with a "NULL" D3D device, you
  // *must* release all D3D resources you eventually allocated
  STDMETHOD(SetDevice)(IDirect3DDevice9 *dev) = 0;

  // "ClearBackground" is called after madVR cleared the render target
  // "RenderOsd" is called after madVR is fully done with rendering
  // fullOutputRect  = (0, 0, outputSurfaceWidth, outputSurfaceHeight)
  // activeVideoRect = active video rendering rect inside of fullOutputRect
  // background area = the part of fullOutputRect which isn't covered by activeVideoRect
  STDMETHOD(ClearBackground)(LPCSTR name, REFERENCE_TIME frameStart, RECT *fullOutputRect, RECT *activeVideoRect) = 0;
  STDMETHOD(RenderOsd      )(LPCSTR name, REFERENCE_TIME frameStart, RECT *fullOutputRect, RECT *activeVideoRect) = 0;
};

// this is the main interface which madVR provides to you
[uuid("3AE03A88-F613-4BBA-AD3E-EE236976BF9A")]
interface IMadVROsdServices : public IUnknown
{
  // this API provides the (1) bitmap based method
  STDMETHOD(OsdSetBitmap)(
    LPCSTR name,                           // name of the OSD element, e.g. "YourMediaPlayer.SeekBar"
    HBITMAP leftEye = NULL,                // OSD bitmap, should be 24bit or 32bit, NULL deletes the OSD element
    HBITMAP rightEye = NULL,               // specify when your OSD is 3D, otherwise set to NULL
    COLORREF colorKey = 0,                 // transparency color key, set to 0 if your bitmap has an 8bit alpha channel
    int posX = 0,                          // where to draw the OSD element?
    int posY = 0,                          //
    bool posRelativeToVideoRect = false,   // draw relative to TRUE: the active video rect; FALSE: the full output rect
    int zOrder = 0,                        // high zOrder OSD elements are drawn on top of those with smaller zOrder values
    DWORD duration = 0,                    // how many milliseconds shall the OSD element be shown (0 = infinite)?
    DWORD flags = 0,                       // undefined - set to 0
    OSDMOUSECALLBACK callback = NULL,      // optional callback for mouse events
    LPVOID callbackContext = NULL,         // this context is passed to the callback
    LPVOID reserved = NULL                 // undefined - set to NULL
  ) = 0;

  // this API allows you to ask the current video rectangles
  STDMETHOD(OsdGetVideoRects)(
    RECT *fullOutputRect,                  // (0, 0, outputSurfaceWidth, outputSurfaceHeight)
    RECT *activeVideoRect                  // active video rendering rect inside of fullOutputRect
  ) = 0;

  // this API provides the (2) render callback based method
  STDMETHOD(OsdSetRenderCallback)(
    LPCSTR name,                           // name of the OSD callback, e.g. "YourMediaPlayer.OsdCallbacks"
    IOsdRenderCallback *callback = NULL,   // OSD callback interface, set to NULL to unregister the callback
    LPVOID reserved = NULL                 // undefined - set to NULL
  ) = 0;

  // this API allows you to force madVR to redraw the current video frame
  // useful when using the (2) render callback method, when the graph is paused
  STDMETHOD(OsdRedrawFrame)(void) = 0;
};

// ---------------------------------------------------------------------------
// IMadVRTextOsd
// ---------------------------------------------------------------------------

// This interface allows you to draw simple text messages.
// madVR uses it internally, too, for showing various messages to the user.
// The messages are shown in the top left corner of the video rendering window.
// The messages work in both windowed and fullscreen exclusive mode.
// There can always be only one message active at the same time, so basically
// the messages are overwriting each other.

[uuid("ABA34FDA-DD22-4E00-9AB4-4ABF927D0B0C")]
interface IMadVRTextOsd : public IUnknown
{
  STDMETHOD(OsdDisplayMessage)(LPCWSTR text, DWORD milliseconds) = 0;
  STDMETHOD(OsdClearMessage)(void) = 0;
};

// ---------------------------------------------------------------------------
// IMadVRSubclassReplacement
// ---------------------------------------------------------------------------

// Normally madVR subclasses some parent of the madVR rendering window.
// If your media player gets into stability issues because of that, you can
// disable madVR's subclassing by using this interface. You should then
// manually forward the messages from your own WindowProc to madVR by calling
// this interface's "ParentWindowProc" method.
// If "ParentWindowProc" returns "TRUE", you should consider the message
// handled by madVR and *not* pass it on to the original WindowProc. Instead
// just return the value madVR wrote to "result". If "ParentWindowProc"
// returns "FALSE", process the message as usual.
// When using the normal subclassing solution, madVR selects the parent window
// to subclass by using the following code:
// {
//   HWND parentWindow = madVRWindow;
//   while ((GetParent(parentWindow)) && (GetParent(parentWindow) == GetAncestor(parentWindow, GA_PARENT)))
//     parentWindow = GetParent(parentWindow);
// }
// If you use this interface, send the messages to madVR from the same window
// that madVR would otherwise have subclassed.

[uuid("9B517604-2D86-4FA2-A20C-ECF88301B010")]
interface IMadVRSubclassReplacement : public IUnknown
{
  STDMETHOD(DisableSubclassing)(void) = 0;
  STDMETHOD_(BOOL, ParentWindowProc)(HWND hwnd, UINT uMsg, WPARAM *wParam, LPARAM *lParam, LRESULT *result) = 0;
};

// ---------------------------------------------------------------------------
// IMadVRSeekbarControl
// ---------------------------------------------------------------------------

// if you draw your own seekbar and absolutely insist on disliking madVR's
// own seekbar, you can forcefully hide it by using this interface
// using this interface only affects the current madVR instance

[uuid("D2D3A520-7CFA-46EB-BA3B-6194A028781C")]
interface IMadVRSeekbarControl : public IUnknown
{
  STDMETHOD(DisableSeekbar)(BOOL disable) = 0;
};

// ---------------------------------------------------------------------------
// IMadVRExclusiveModeControl
// ---------------------------------------------------------------------------

// you can use this interface to turn madVR's automatic exclusive mode on/off
// using this interface only affects the current madVR instance

[uuid("88A69329-3CD3-47D6-ADEF-89FA23AFC7F3")]
interface IMadVRExclusiveModeControl : public IUnknown
{
  STDMETHOD(DisableExclusiveMode)(BOOL disable) = 0;
};

// ---------------------------------------------------------------------------
// IMadVRExclusiveModeCallback
// ---------------------------------------------------------------------------

// allows you to be notified when exclusive mode is entered/left

#define ExclusiveModeIsAboutToBeEntered 1
#define ExclusiveModeWasJustEntered     2
#define ExclusiveModeIsAboutToBeLeft    3
#define ExclusiveModeWasJustLeft        4
typedef void (__stdcall *EXCLUSIVEMODECALLBACK)(LPVOID context, int event);

[uuid("51CA9252-ACC5-4EC5-A02E-0F9F8C42B536")]
interface IMadVRExclusiveModeCallback : public IUnknown
{
  STDMETHOD(  Register)(EXCLUSIVEMODECALLBACK exclusiveModeCallback, LPVOID context) = 0;
  STDMETHOD(Unregister)(EXCLUSIVEMODECALLBACK exclusiveModeCallback, LPVOID context) = 0;
};

// ---------------------------------------------------------------------------
// IMadVRDirect3D9Manager
// ---------------------------------------------------------------------------

// You can make madVR use your Direct3D9 device(s) instead of creating its
// own. If you do that, madVR will not automatically switch between
// fullscreen and windowed mode, anymore. It's your duty then to enter
// fullscreen exclusive mode, if you want madVR to use it. madVR will still
// reset the devices, though, if necessary (lost device etc).

[uuid("1CAEE23B-D14B-4DB4-8AEA-F3528CB78922")]
interface IMadVRDirect3D9Manager : public IUnknown
{
  // Creating 3 different devices for scanline reading, rendering and
  // presentation can improve overall performance. If you don't like that
  // idea, just feed madVR with the same device for all tasks.
  // You can't create new devices if one device is already in fullscreen
  // exclusive mode. So if you want to provide madVR with 3 different
  // devices, while still using exclusive mode, make sure you create the
  // scanline reading and rendering devices before setting the presentation
  // device to fullscreen exclusive mode.
  STDMETHOD(UseTheseDevices)(LPDIRECT3DDEVICE9 scanlineReading, LPDIRECT3DDEVICE9 rendering, LPDIRECT3DDEVICE9 presentation) = 0;

  // madVR contains a display mode changer which, depending on the video
  // size and frame rate, may decide to switch display modes. If you don't
  // want madVR to change either resolution or refresh rates, you can
  // disable this functionality partially or completely.
  STDMETHOD(ConfigureDisplayModeChanger)(BOOL allowResolutionChanges, BOOL allowRefreshRateChanges) = 0;
};

// ---------------------------------------------------------------------------
// IMadVRExternalPixelShaders
// ---------------------------------------------------------------------------

// this interface allows you to activate external HLSL D3D9 pixel shaders

#define ShaderStage_PreScale 0
#define ShaderStage_PostScale 1

[uuid("B6A6D5D4-9637-4C7D-AAAE-BC0B36F5E433")]
interface IMadVRExternalPixelShaders : public IUnknown
{
  STDMETHOD(ClearPixelShaders)(int stage) = 0;
  STDMETHOD(AddPixelShader)(LPCSTR sourceCode, LPCSTR compileProfile, int stage, LPVOID reserved) = 0;
};

// ---------------------------------------------------------------------------
// IMadVRInfo
// ---------------------------------------------------------------------------

// this interface allows you to get all kinds of information from madVR

[uuid("8FAB7F31-06EF-444C-A798-10314E185532")]
interface IMadVRInfo : public IUnknown
{
  // The memory for strings and binary data is allocated by the callee
  // by using LocalAlloc. It is the caller's responsibility to release the
  // memory by calling LocalFree.
  // Field names and LPWSTR values should be read case insensitive.
  STDMETHOD(GetBool     )(LPCSTR field, bool      *value) = 0;
  STDMETHOD(GetInt      )(LPCSTR field, int       *value) = 0;
  STDMETHOD(GetSize     )(LPCSTR field, SIZE      *value) = 0;
  STDMETHOD(GetRect     )(LPCSTR field, RECT      *value) = 0;
  STDMETHOD(GetUlonglong)(LPCSTR field, ULONGLONG *value) = 0;
  STDMETHOD(GetDouble   )(LPCSTR field, double    *value) = 0;
  STDMETHOD(GetString   )(LPCSTR field, LPWSTR    *value, int *chars) = 0;
  STDMETHOD(GetBin      )(LPCSTR field, LPVOID    *value, int *size ) = 0;
};

// available info fields:
// ----------------------
// version,                 string,    madVR version number
// originalVideoSize,       size,      size of the video before scaling and AR adjustments
// arAdjustedVideoSize,     size,      size of the video after AR adjustments
// videoOutputRect,         rect,      final pos/size of the video after all scaling operations
// subtitleTargetRect,      rect,      consumer wish for where to place the subtitles
// frameRate,               ulonglong, frame rate of the video after deinterlacing (REFERENCE_TIME)
// refreshRate,             double,    display refresh rate (0, if unknown)
// displayModeSize,         size,      display mode width/height
// yuvMatrix,               string,    RGB Video: "None" (fullrange); YCbCr Video: "Levels.Matrix", Levels: TV|PC, Matrix: 601|709|240M|FCC|2020
// exclusiveModeActive,     bool,      is madVR currently in exclusive mode?
// madVRSeekbarEnabled,     bool,      is the madVR exclusive mode seek bar currently enabled?
// dxvaDecodingActive,      bool,      is DXVA2 decoding      being used at the moment?
// dxvaDeinterlacingActive, bool,      is DXVA2 deinterlacing being used at the moment?
// dxvaScalingActive,       bool,      is DXVA2 scaling       being used at the moment?
// ivtcActive,              bool,      is madVR's IVTC algorithm active at the moment?
// osdLatency,              int,       how much milliseconds will pass for an OSD change to become visible?

// ---------------------------------------------------------------------------
// IMadVRSettings
// ---------------------------------------------------------------------------

// this interface allows you to read and write madVR settings

// For each folder and value there exists both a short ID and a long
// description. The short ID will never change. The long description may be
// modified in a future version. So it's preferable to use the ID, but you can
// also address settings by using the clear text description.

// The "path" parameter can simply be set to the ID or to the description of
// the setting value. Alternatively you can use a partial or full path to the
// setting value. E.g. the following calls will all return the same value:
// (1) GetBoolean(L"dontDither", &boolVal);
// (2) GetBoolean(L"don't use dithering", &boolVal);
// (3) GetBoolean(L"dithering\dontDither", &boolVal);
// (4) GetBoolean(L"rendering\dithering\dontDither", &boolVal);

// Using the full path can make sense if you want to access a specific profile.
// If you don't specify a path, you automatically access the currently active
// profile.

[uuid("6F8A566C-4E19-439E-8F07-20E46ED06DEE")]
interface IMadVRSettings : public IUnknown
{
  // returns the revision number of the settings record
  // the revision number is increased by 1 every time a setting changes
  STDMETHOD_(BOOL, SettingsGetRevision)(LONGLONG* revision) = 0;

  // export the whole settings record to a binary data buffer
  // the buffer is allocated by SettingsExport by using LocalAlloc
  // it's the caller's responsibility to free the buffer again by using LocalFree
  STDMETHOD_(BOOL, SettingsExport)(LPVOID* buf, int* size) = 0;
  // import the settings from a binary data buffer
  STDMETHOD_(BOOL, SettingsImport)(LPVOID buf, int size) = 0;

  // modify a specific value
  STDMETHOD_(BOOL, SettingsSetString )(LPCWSTR path, LPCWSTR value) = 0;
  STDMETHOD_(BOOL, SettingsSetInteger)(LPCWSTR path, int     value) = 0;
  STDMETHOD_(BOOL, SettingsSetBoolean)(LPCWSTR path, BOOL    value) = 0;

  // The buffer for SettingsGetString must be provided by the caller and
  // bufLenInChars set to the buffer's length (please note: 1 char -> 2 bytes).
  // If the buffer is too small, the API fails and GetLastError returns
  // ERROR_MORE_DATA. On return, bufLenInChars is set to the required buffer size.
  // The buffer for SettingsGetBinary is allocated by SettingsGetBinary.
  // The caller is responsible for freeing it by using LocalAlloc().
  STDMETHOD_(BOOL, SettingsGetString )(LPCWSTR path, LPCWSTR value, int* bufLenInChars) = 0;
  STDMETHOD_(BOOL, SettingsGetInteger)(LPCWSTR path, int*    value) = 0;
  STDMETHOD_(BOOL, SettingsGetBoolean)(LPCWSTR path, BOOL*   value) = 0;
  STDMETHOD_(BOOL, SettingsGetBinary )(LPCWSTR path, LPVOID* value, int* bufLenInBytes) = 0;
};

[uuid("1C3E03D6-F422-4D31-9424-75936F663BF7")]
interface IMadVRSettings2 : public IMadVRSettings
{
  // Enumerate the available settings stuff in the specified path.
  // Simply loop from enumIndex 0 to infinite, until the enumeration returns FALSE.
  // When enumeration is completed GetLastError returns ERROR_NO_MORE_ITEMS.
  // The buffers must be provided by the caller and ...LenInChars set to the
  // buffer's length (please note: 1 char -> 2 bytes). If the buffer is too small,
  // the API fails and GetLastError returns ERROR_MORE_DATA. On return,
  // ...LenInChars is set to the required buffer size.
  STDMETHOD_(BOOL, SettingsEnumFolders      )(LPCWSTR path, int enumIndex, LPCWSTR id, LPCWSTR name, LPCWSTR type, int* idLenInChars, int* nameLenInChars, int* typeLenInChars) = 0;
  STDMETHOD_(BOOL, SettingsEnumValues       )(LPCWSTR path, int enumIndex, LPCWSTR id, LPCWSTR name, LPCWSTR type, int* idLenInChars, int* nameLenInChars, int* typeLenInChars) = 0;
  STDMETHOD_(BOOL, SettingsEnumProfileGroups)(LPCWSTR path, int enumIndex,             LPCWSTR name,                                  int* nameLenInChars                     ) = 0;
  STDMETHOD_(BOOL, SettingsEnumProfiles     )(LPCWSTR path, int enumIndex,             LPCWSTR name,                                  int* nameLenInChars                     ) = 0;

  // Creates/deletes a profile group in the specified path.
  // Deleting a profile group works only if there's only one profile left in the group.
  // Example:
  // SettingsCreateProfileGroup('scalingParent', 'imageDoubling|lumaUp', 'upscaling profiles', 'SD 24fps');
  // SettingsDeleteProfileGroup('scalingParent\upscaling profiles');
  STDMETHOD_(BOOL, SettingsCreateProfileGroup)(LPCWSTR path, LPCWSTR settingsPageList, LPCWSTR profileGroupName, LPCWSTR firstProfileName) = 0;
  STDMETHOD_(BOOL, SettingsDeleteProfileGroup)(LPCWSTR path) = 0;

  // SettingsAddProfile adds a new profile, using default parameters for all values.
  // SettingsDuplicateProfile duplicates/copies a profile with all parameters.
  // Deleting a profile works only if it isn't the only profile left in the group.
  // Example:
  // SettingsAddProfile('scalingParent\upscaling profiles', 'SD 60fps');
  // SettingsDuplicateProfile('scalingParent\upscaling profiles', 'SD 60fps', 'HD 24fps');
  // SettingsDeleteProfile('scalingParent\upscaling profiles', 'SD 60fps');
  STDMETHOD_(BOOL, SettingsAddProfile      )(LPCWSTR path,                              LPCWSTR newProfileName) = 0;
  STDMETHOD_(BOOL, SettingsDuplicateProfile)(LPCWSTR path, LPCWSTR originalProfileName, LPCWSTR newProfileName) = 0;
  STDMETHOD_(BOOL, SettingsDeleteProfile   )(LPCWSTR path, LPCWSTR         profileName                        ) = 0;

  // SettingsActivateProfile activates the specified profile.
  // It also disables automatic (rule based) profile selection.
  // SettingsAutoselectProfile allows you to reactivate it.
  // Example:
  // if SettingsIsProfileActive('scalingParent\upscaling profiles', 'SD 24fps') then
  // begin
  //   SettingsActivateProfile('scalingParent\upscaling profiles', 'SD 60fps');
  //   [...]
  //   SettingsAutoselectProfile('scalingParent\upscaling profiles');
  STDMETHOD_(BOOL, SettingsIsProfileActive)(LPCWSTR path, LPCWSTR profileName) = 0;
  STDMETHOD_(BOOL, SettingsActivateProfile)(LPCWSTR path, LPCWSTR profileName) = 0;
  STDMETHOD_(BOOL, SettingsIsProfileAutoselected)(LPCWSTR path) = 0;
  STDMETHOD_(BOOL, SettingsAutoselectProfile)(LPCWSTR path) = 0;
};

// available settings: id, name, type, valid values
// ------------------------------------------------
// devices, devices
//   %monitorId%, %monitorName%
//     %id%, identification
//       edid,                      edid,                                                               binary
//       monitorName,               monitor name,                                                       string
//       deviceId,                  device id,                                                          string
//       outputDevice,              output device,                                                      string
//     properties, properties
//       levels,                    levels,                                                             string,  TV Levels|PC Levels|Custom
//       black,                     black,                                                              integer, 0..48
//       white,                     white,                                                              integer, 200..255
//       displayBitdepth,           native display bitdepth,                                            integer, 3..10
//     calibration, calibration
//       calibrate,                 calibrate display,                                                  string,  disable calibration controls for this display|this display is already calibrated|calibrate this display by using yCMS|calibrate this display by using an external 3dlut file
//       disableGpuGammaRamps,      disable GPU gamma ramps,                                            boolean
//       external3dlutFile,         external 3dlut file,                                                string
//       gamutMeasurements,         gamut measurements,                                                 string
//       gammaMeasurements,         gamma measurements,                                                 string
//       displayPrimaries,          display primaries,                                                  string,  BT.709 (HD)|BT.601 (SD)|PAL|something else
//       displayGammaCurve,         display gamma curve,                                                string,  pure power curve|BT.709/601 curve|something else
//       displayGammaValue,         display gamma value,                                                string,  1.80|1.85|1.90|1.95|2.00|2.05|2.10|2.15|2.20|2.25|2.30|2.35|2.40|2.45|2.50|2.55|2.60|2.65|2.70|2.75|2.80
//     displayModes, display modes
//       enableDisplayModeChanger,  switch to matching display mode...,                                 boolean
//       changeDisplayModeOnPlay,   ... when playback starts,                                           boolean
//       restoreDisplayMode,        restore original display mode...,                                   boolean
//       restoreDisplayModeOnClose, ... when media player is closed,                                    boolean
//       slowdown,                  treat 25p movies as 24p  (requires Reclock),                        boolean
//       displayModesData,          display modes data,                                                 binary
//     colorGamma, color & gamma
//       brightness,                brightness,                                                         integer, -100..+100
//       contrast,                  contrast,                                                           integer, -100..+100
//       saturation,                saturation,                                                         integer, -100..+100
//       hue,                       hue,                                                                integer, -180..+180
//       enableGammaProcessing,     enable gamma processing,                                            boolean
//       currentGammaCurve,         current gamma curve,                                                string,  pure power curve|BT.709/601 curve
//       currentGammaValue,         current gamma value,                                                string,  1.80|1.85|1.90|1.95|2.00|2.05|2.10|2.15|2.20|2.25|2.30|2.35|2.40|2.45|2.50|2.55|2.60|2.65|2.70|2.75|2.80
// processing, processing
//   decoding, processing
//     decodeH264,                  decode h264,                                                        string,  disable|libav|intel|hardware
//     decodeVc1,                   decode VC-1,                                                        string,  disable|libav|intel|hardware
//     decodeMpeg2,                 decode MPEG2,                                                       string,  disable|libav|intel|hardware
//   deinterlacing, deinterlacing
//     autoActivateDeinterlacing,   automatically activate deinterlacing when needed,                   boolean
//     ifInDoubtDeinterlace,        if in doubt, activate deinterlacing,                                boolean
//     contentType,                 source type,                                                        string,  auto|film|video
//     scanPartialFrame,            only look at pixels in the frame center,                            boolean
//     deinterlaceThread,           perform deinterlacing in separate thread,                           boolean
//   artifactRemoval, artifact removal
//     debandActive,                reduce banding artifacts,                                           boolean
//     debandLevel,                 default debanding strength,                                         integer, 0..2
//     debandFadeLevel,             strength during fade in/out,                                        integer, 0..2
// scalingParent, scaling algorithms
//   chromaUp, chroma upscaling
//     chromaUp,                    chroma upsampling,                                                  string,  Nearest Neighbor|Bilinear|Mitchell-Netravali|Catmull-Rom|Bicubic50|Bicubic60|Bicubic75|Bicubic100|SoftCubic50|SoftCubic60|SoftCubic70|SoftCubic80|SoftCubic100|Lanczos3|Lanczos4|Lanczos8|Spline36|Spline64|Jinc3|Jinc4|Jinc8|Nnedi16|Nnedi32|Nnedi64|Nnedi128|Nnedi256
//     chromaAntiRinging,           activate anti-ringing filter for chroma upsampling,                 boolean
//   imageDoubling, image doubling
//     nnediDLEnable,               use NNEDI3 to double Luma resolution,                               boolean
//     nnediDCEnable,               use NNEDI3 to double Chroma resolution,                             boolean
//     nnediQLEnable,               use NNEDI3 to quadruple Luma resolution,                            boolean
//     nnediQCEnable,               use NNEDI3 to quadruple Chroma resolution,                          boolean
//     nnediDLScalingFactor,        when to use NNEDI3 to double Luma resolution,                       string,  1.2x|1.5x|2.0x|always
//     nnediDCScalingFactor,        when to use NNEDI3 to double Chroma resolution,                     string,  1.2x|1.5x|2.0x|always
//     nnediQLScalingFactor,        when to use NNEDI3 to quadruple Luma resolution,                    string,  1.2x|1.5x|2.0x|always
//     nnediQCScalingFactor,        when to use NNEDI3 to quadruple Chroma resolution,                  string,  1.2x|1.5x|2.0x|always
//     nnediDLQuality,              NNEDI3 double Luma quality,                                         integer, 0..4
//     nnediDLQuality,              NNEDI3 double Chroma quality,                                       integer, 0..4
//     nnediDLQuality,              NNEDI3 quadruple Luma quality,                                      integer, 0..4
//     nnediDLQuality,              NNEDI3 quadruple Chroma quality,                                    integer, 0..4
//     amdInteropHack,              use alternative interop hack (not recommended, AMD only),           boolean
//   lumaUp, image upscaling
//     lumaUp,                      image upscaling,                                                    string,  Nearest Neighbor|Bilinear|Dxva|Mitchell-Netravali|Catmull-Rom|Bicubic50|Bicubic60|Bicubic75|Bicubic100|SoftCubic50|SoftCubic60|SoftCubic70|SoftCubic80|SoftCubic100|Lanczos3|Lanczos4|Lanczos8|Spline36|Spline64|Jinc3|Jinc4|Jinc8
//     lumaUpAntiRinging,           activate anti-ringing filter for luma upsampling,                   boolean
//     lumaUpLinear,                upscale luma in linear light,                                       boolean
//   lumaDown, image downscaling
//     lumaDown,                    image downscaling,                                                  string,  Nearest Neighbor|Bilinear|Dxva|Mitchell-Netravali|Catmull-Rom|Bicubic50|Bicubic60|Bicubic75|Bicubic100|SoftCubic50|SoftCubic60|SoftCubic70|SoftCubic80|SoftCubic100|Lanczos3|Lanczos4|Lanczos8|Spline36|Spline64
//     lumaDownAntiRinging,         activate anti-ringing filter for luma downsampling,                 boolean
//     lumaDownLinear,              downscale luma in linear light,                                     boolean
// rendering, rendering
//   basicRendering, general settings
//     managedUpload,               use managed upload textures (XP only),                              boolean
//     uploadInRenderThread,        upload frames in render thread,                                     boolean
//     delayPlaybackStart2,         delay playback start until render queue is full,                    boolean
//     delaySeek,                   delay playback start after seeking, too,                            boolean
//     enableOverlay,               enable windowed overlay (Windows 7 and newer),                      boolean
//     enableExclusive,             enable automatic fullscreen exclusive mode,                         boolean
//     disableAero,                 disable desktop composition (Vista and newer),                      boolean
//     disableAeroCfg,              disable desktop composition configuration,                          string,  during exclusive - windowed mode switch|while madVR is in exclusive mode|while media player is in fullscreen mode|always
//     splitNv12ViaOpenCL,          use OpenCL to process DXVA NV12 surfaces (Intel, AMD),              boolean
//     separateDevice,              use a separate device for presentation (Vista and newer),           boolean
//     useD3d11,                    use D3D11 for presentation,                                         boolean
//     dxvaDevice,                  use separate device for DXVA processing (Vista and newer),          boolean
//     cpuQueueSize,                CPU queue size,                                                     integer, 4..32
//     gpuQueueSize,                GPU queue size,                                                     integer, 4..24
//   windowedTweaks, windowed mode settings
//     backbufferCount,             no of backbuffers,                                                  integer, 1..8
//     flushAfterRenderSteps,       after render steps,                                                 string,  don''t flush|flush|flush & wait (sleep)|flush & wait (loop)
//     flushAfterLastStep,          after last step,                                                    string,  don''t flush|flush|flush & wait (sleep)|flush & wait (loop)
//     flushAfterBackbuffer,        after backbuffer,                                                   string,  don''t flush|flush|flush & wait (sleep)|flush & wait (loop)
//     flushAfterPresent,           after present,                                                      string,  don''t flush|flush|flush & wait (sleep)|flush & wait (loop)
//     oldWindowedPath,             use old windowed rendering path,                                    boolean
//     preRenderFramesWindowed,     no of pre-presented frames,                                         integer, 1..16
//   exclusiveSettings, exclusive mode settings
//     enableSeekbar,               show seek bar,                                                      boolean
//     exclusiveDelay,              delay switch to exclusive mode by 3 seconds,                        boolean
//     oldExclusivePath,            use old fse rendering path,                                         boolean
//     presentThread,               run presentation in a separate thread,                              boolean
//     avoidGlitches,               limit rendering times to avoid glitches,                            boolean
//     overshootMaxLatency,         overshoot max frame latency (Vista and newer),                      boolean
//     preRenderFrames,             no of pre-presented frames,                                         integer, 1..16
//     backbufferCountExcl,         no of backbuffers,                                                  integer, 1..8
//     flushAfterRenderStepsExcl,   after render steps,                                                 string,  don''t flush|flush|flush & wait (sleep)|flush & wait (loop)
//     flushAfterLastStepExcl,      after last step,                                                    string,  don''t flush|flush|flush & wait (sleep)|flush & wait (loop)
//     flushAfterBackbufferExcl,    after backbuffer,                                                   string,  don''t flush|flush|flush & wait (sleep)|flush & wait (loop)
//     flushAfterPresentExcl,       after present,                                                      string,  don''t flush|flush|flush & wait (sleep)|flush & wait (loop)
//   smoothMotion, smooth motion
//     smoothMotionEnabled,         enable smooth motion frame rate conversion,                         boolean
//     smoothMotionMode,            smooth motion mode,                                                 string,  avoidJudder|almostAlways|always
//   dithering, dithering
//     ditheringAlgo,               dithering algorithm,                                                string,  random|ordered|errorDifLowNoise|errorDifMedNoise
//     dontDither,                  don't use dithering,                                                boolean
//     coloredDither,               use colored noise,                                                  boolean
//     dynamicDither,               change dither for every frame,                                      boolean
//   tradeQuality, trade quality for performance
//     fastSubtitles,               optimize subtitles for performance instead of quality,              boolean
//     customShaders16f,            store custom pixel shader results in 16bit buffer instead of 32bit, boolean
//     gammaDithering,              don't use linear light for dithering,                               boolean
//     noGradientAngles,            don't analyze gradient angles for debanding,                        boolean
//     dontRerenderFades,           don't rerender frames when fade in/out is detected,                 boolean
//     noDeintCopyback,             don't use "copyback" for DXVA deinterlacing,                        boolean
//     noDecodeCopyback,            don't use "copyback" for DXVA decoding,                             boolean
//     gammaBlending,               don't use linear light for smooth motion frame blending,            boolean
//     10bitChroma,                 use 10bit chroma buffer instead of 16bit,                           boolean
//     10bitLuma,                   use 10bit image buffer instead of 16bit,                            boolean
//     customShadersTv,             run custom pixel shaders in video levels instead of PC levels,      boolean
//     3dlutLowerBitdepth,          use lower bitdepth for yCMS 3dlut calibration,                      boolean
//     3dlutBitdepth,               3dlut bitdepth,                                                     integer, 6..7
//     halfDxvaDeintFramerate,      use half frame rate for DXVA deinterlacing,                         boolean
// ui, user interface
//   keys, keyboard shortcuts
//     keysOnlyIfFocused,           use only if media player has keyboard focus,                        boolean
//     keyDebugOsd,                 debug OSD - toggle on/off,                                          string
//     keyResetStats,               debug OSD - reset statistics,                                       string
//     keyFreezeReport,             create freeze report,                                               string
//     keyOutputLevels,             output levels - toggle,                                             string
//     keySourceLevels,             source levels - toggle,                                             string
//     keySourceBlackInc,           source black level - increase,                                      string
//     keySourceBlackDec,           source black level - decrease,                                      string
//     keySourceWhiteInc,           source white level - increase,                                      string
//     keySourceWhiteDec,           source white level - decrease,                                      string
//     keySourceBrightnessInc,      source brightness - increase,                                       string
//     keySourceBrightnessDec,      source brightness - decrease,                                       string
//     keySourceContrastInc,        source contrast - increase,                                         string
//     keySourceContrastDec,        source contrast - decrease,                                         string
//     keySourceSaturationInc,      source saturation - increase,                                       string
//     keySourceSaturationDec,      source saturation - decrease,                                       string
//     keySourceHueInc,             source hue - increase,                                              string
//     keySourceHueDec,             source hue - decrease,                                              string
//     keySourceColorControlReset,  source color control - reset,                                       string
//     keyMatrix,                   source decoding matrix - toggle,                                    string
//     keyPrimaries,                source primaries - toggle,                                          string
//     keyPrimariesEbu,             source primaries - set to "EBU/PAL",                                string
//     keyPrimaries709,             source primaries - set to "BT.709",                                 string
//     keyPrimariesSmpteC,          source primaries - set to "SMPTE C",                                string
//     keyPrimaries2020,            source primaries - set to "BT.2020",                                string
//     keyPrimariesDci,             source primaries - set to "DCI-P3",                                 string
//     keyDeint,                    deinterlacing - toggle,                                             string
//     keyDeintFieldOrder,          deinterlacing field order - toggle,                                 string
//     keyDeintContentType,         deinterlacing content type - toggle,                                string
//     keyDeintContentTypeFilm,     deinterlacing content type - set to "film",                         string
//     keyDeintContentTypeVideo,    deinterlacing content type - set to "video",                        string
//     keyDeintContentTypeAuto,     deinterlacing content type - set to "auto detect",                  string
//     keyDeband,                   debanding - toggle,                                                 string
//     keyDebandCustom,             debanding custom settings - toggle,                                 string
//     keyDesiredGammaCurve,        desired display gamma curve - toggle,                               string
//     keyDesiredGammaValueInc,     desired display gamma value - increase,                             string
//     keyDesiredGammaValueDec,     desired display gamma value - decrease,                             string
//     keyFseEnable,                automatic fullscreen exclusive mode - enable,                       string
//     keyFseDisable,               automatic fullscreen exclusive mode - disable,                      string
//     keyFseDisable10,             automatic fullscreen exclusive mode - disable for 10 seconds,       string
//     keyEnableSmoothMotion,       enable smooth motion frame rate conversion,                         string
//     keyDisableSmoothMotion,      disable smooth motion frame rate conversion,                        string
//     keyChromaAlgo,               chroma upscaling algorithm - toggle,                                string
//     keyChromaAlgoNearest,        chroma upscaling algorithm - set to "Nearest Neighbor",             string
//     keyChromaAlgoBilinear,       chroma upscaling algorithm - set to "Bilinear",                     string
//     keyChromaAlgoMitchell,       chroma upscaling algorithm - set to "Mitchell-Netravali",           string
//     keyChromaAlgoCatmull,        chroma upscaling algorithm - set to "Catmull-Rom",                  string
//     keyChromaAlgoBicubic,        chroma upscaling algorithm - set to "Bicubic",                      string
//     keyChromaAlgoSoftCubic,      chroma upscaling algorithm - set to "SoftCubic",                    string
//     keyChromaAlgoLanczos,        chroma upscaling algorithm - set to "Lanczos",                      string
//     keyChromaAlgoSpline,         chroma upscaling algorithm - set to "Spline",                       string
//     keyChromaAlgoJinc,           chroma upscaling algorithm - set to "Jinc",                         string
//     keyChromaAlgoParamInc,       chroma upscaling algorithm parameter - increase,                    string
//     keyChromaAlgoParamDec,       chroma upscaling algorithm parameter - decrease,                    string
//     keyChromaAntiRing,           chroma upscaling anti-ringing filter - toggle on/off,               string
//     keyImageUpAlgo,              image upscaling algorithm - toggle,                                 string
//     keyImageUpAlgoNearest,       image upscaling algorithm - set to "Nearest Neighbor",              string
//     keyImageUpAlgoBilinear,      image upscaling algorithm - set to "Bilinear",                      string
//     keyImageUpAlgoDxva,          image upscaling algorithm - set to "DXVA2",                         string
//     keyImageUpAlgoMitchell,      image upscaling algorithm - set to "Mitchell-Netravali",            string
//     keyImageUpAlgoCatmull,       image upscaling algorithm - set to "Catmull-Rom",                   string
//     keyImageUpAlgoBicubic,       image upscaling algorithm - set to "Bicubic",                       string
//     keyImageUpAlgoSoftCubic,     image upscaling algorithm - set to "SoftCubic",                     string
//     keyImageUpAlgoLanczos,       image upscaling algorithm - set to "Lanczos",                       string
//     keyImageUpAlgoSpline,        image upscaling algorithm - set to "Spline",                        string
//     keyImageUpAlgoJinc,          image upscaling algorithm - set to "Jinc",                          string
//     keyImageUpAlgoParamInc,      image upscaling algorithm parameter - increase,                     string
//     keyImageUpAlgoParamDec,      image upscaling algorithm parameter - decrease,                     string
//     keyImageUpAntiRing,          image upscaling anti-ringing filter - toggle on/off,                string
//     keyImageUpLinear,            image upscaling in linear light - toggle on/off,                    string
//     keyImageDownAlgo,            image downscaling algorithm - toggle,                               string
//     keyImageDownAlgoNearest,     image downscaling algorithm - set to "Nearest Neighbor",            string
//     keyImageDownAlgoBilinear,    image downscaling algorithm - set to "Bilinear",                    string
//     keyImageDownAlgoDxva,        image downscaling algorithm - set to "DXVA2",                       string
//     keyImageDownAlgoMitchell,    image downscaling algorithm - set to "Mitchell-Netravali",          string
//     keyImageDownAlgoCatmull,     image downscaling algorithm - set to "Catmull-Rom",                 string
//     keyImageDownAlgoBicubic,     image downscaling algorithm - set to "Bicubic",                     string
//     keyImageDownAlgoSoftCubic,   image downscaling algorithm - set to "SoftCubic",                   string
//     keyImageDownAlgoLanczos,     image downscaling algorithm - set to "Lanczos",                     string
//     keyImageDownAlgoSpline,      image downscaling algorithm - set to "Spline",                      string
//     keyImageDownAlgoParamInc,    image downscaling algorithm parameter - increase,                   string
//     keyImageDownAlgoParamDec,    image downscaling algorithm parameter - decrease,                   string
//     keyImageDownAntiRing,        image downscaling anti-ringing filter - toggle on/off,              string
//     keyImageDownLinear,          image downscaling in linear light - toggle on/off,                  string
//     keyDisplayModeChanger,       display mode switcher - toggle on/off,                              string
//     keyDisplayBitdepth,          display bitdepth - toggle,                                          string
//     keyDithering,                dithering - toggle on/off,                                          string

// profile settings: id, name, type, valid values
// ----------------------------------------------
// Profile Group 1
//   keyToggleProfiles,             keyboard shortcut to toggle profiles,                               string
//   autoselectRules,               profile auto select rules,                                          string
//   Profile 1
//     keyActivateProfile,          keyboard shortcut to activate this profile,                         string
//     activateCmdline,             command line to execute when this profile is activated,             string
//     deactivateCmdline,           command line to execute when this profile is deactivated,           string

// ---------------------------------------------------------------------------
// IMadVRExclusiveModeInfo (obsolete)
// ---------------------------------------------------------------------------

// CAUTION: This interface is obsolete. Use IMadVRInfo instead:
// IMadVRInfo::InfoGetBoolean("ExclusiveModeActive")
// IMadVRInfo::InfoGetBoolean("MadVRSeekbarEnabled")

// this interface allows you to ask...
// ... whether madVR is currently in exclusive mode
// ... whether the madVR exclusive mode seek bar is currently enabled

// If madVR is in fullscreen exclusive mode, you should be careful with
// which GUI you show, because showing any window based GUI will make madVR
// automatically switch back to windowed mode. That's ok if that's what you
// really want, just be aware of that. A good alternative is to use the
// graphical or text base OSD interfaces (see above). Using them instead of
// a window based GUI means that madVR can stay in exclusive mode all the
// time.

// Since madVR has its own seek bar (which is only shown in fullscreen
// exclusive mode, though), before showing your own seek bar you should
// check whether madVR is in fullscreen exclusive mode and whether the
// user has enabled madVR's own seek bar. If so, you should probably not
// show your own seek bar. If the user, however, has the madVR seek bar
// disabled, you should still show your own seek bar, because otherwise
// the user will have no way to seek at all.

[uuid("D6EE8031-214E-4E9E-A3A7-458925F933AB")]
interface IMadVRExclusiveModeInfo : public IUnknown
{
  STDMETHOD_(BOOL, IsExclusiveModeActive)(void) = 0;
  STDMETHOD_(BOOL, IsMadVRSeekbarEnabled)(void) = 0;
};

// ---------------------------------------------------------------------------
// IMadVRRefreshRateInfo (obsolete)
// ---------------------------------------------------------------------------

// CAUTION: This interface is obsolete. Use IMadVRInfo instead:
// IMadVRInfo::InfoGetDouble("RefreshRate")

// this interface allows you to ask madVR about the detected refresh rate

[uuid("3F6580E8-8DE9-48D0-8E4E-1F26FE02413E")]
interface IMadVRRefreshRateInfo : public IUnknown
{
  STDMETHOD(GetRefreshRate)(double *refreshRate) = 0;
};

// ---------------------------------------------------------------------------

#endif // __mvrInterfaces__
