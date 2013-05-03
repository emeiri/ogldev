/* Copyright (c) 2011, Max Aizenshtein <max.sniffer@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#pragma once

extern "C" {

/**************************************************
* glfxGenEffect
* Return value: Effect id
**************************************************/
int glfxGenEffect();

/**************************************************
* glfxCreateEffectFromFile
* Input:
*   effect  -- GLFX effect id
*   file    -- File name
* Return value: Status
**************************************************/
bool glfxParseEffectFromFile( int effect, const char* file );

/**************************************************
* glfxCompileProgram
* Input:
*   effect  -- GLFX effect id
*   program -- Program name
* Return value: GL program id if success, -1 otherwise
**************************************************/
int glfxCompileProgram(int effect, const char* program);

/**************************************************
* glfxGenerateSampler
* Input:
*   effect  -- GLFX effect id
*   sampler -- Sampler name
* Return value: GL sampler id if success, -1 otherwise
**************************************************/
int glfxGenerateSampler(int effect, const char* sampler);

/**************************************************
* glfxGetEffectLog
* Input:
*   effect  -- GLFX effect id
*   log     -- Destination address
*   bufSize -- Size of the buffer
**************************************************/
void glfxGetEffectLog(int effect, char* log, int bufSize);

/**************************************************
* glfxDeleteEffect
* Input:
*   effect  -- GLFX effect id
**************************************************/
void glfxDeleteEffect(int effect);

};


#ifdef __cplusplus

#include <string>
/**************************************************
* glfxGetEffectLog
* Input:
*   effect  -- GLFX effect id
* Return value: Log string
**************************************************/
std::string glfxGetEffectLog(int effect);
#endif
