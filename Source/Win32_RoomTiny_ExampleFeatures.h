/************************************************************************************
Filename    :   Win32_RoomTiny_ExampleFeatures.h
Content     :   First-person view test application for Oculus Rift
Created     :   October 20th, 2014
Author      :   Tom Heath
Copyright   :   Copyright 2014 Oculus, Inc. All Rights reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
 
http://www.apache.org/licenses/LICENSE-2.0
 
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*************************************************************************************/

// Note, these options may not work in combination, 
//       and may not apply to both SDK-rendered and APP-rendered

int clock = 0; 

#if !SDK_RENDER
// Part 1 of 5 - Stereo-matching one-eye-per-frame.
// We render only one eye per frame, employing a 3rd buffer, so we can wait until both frames 
// stereoscopically match before presenting them, timewarped to the user.  
// We do this by having 2 buffers for the left eye, so we can hang onto an older version.
// Operate with the 'M' key.
// Non SDK-rendered only.
ImageBuffer * extraEyeRenderTexture; 
ovrPosef      extraRenderPose; 
float         extraYaw;   
ShaderFill  * extraShaderFill;

//Used by some features
void MakeNewDistortionMeshes(float overrideEyeRelief=0);
#endif

//-----------------------------------------------------------------------------------------------------
void ExampleFeatures1(float * pSpeed, int * pTimesToRenderScene, ovrVector3f * useHmdToEyeViewOffset)
{
    // Update the clock, used by some of the features
    clock++;

	// Recenter the Rift by pressing 'R'
    if (DX11.Key['R'])
        ovrHmd_RecenterPose(HMD);

#if SDK_RENDER
    OVR_UNUSED(pSpeed);
    OVR_UNUSED(pTimesToRenderScene);

    // Dismiss the Health and Safety message by pressing any key
    if (DX11.IsAnyKeyPressed())
        ovrHmd_DismissHSWDisplay(HMD);
#endif
}
