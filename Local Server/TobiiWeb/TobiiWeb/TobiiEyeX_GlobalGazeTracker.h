/*
*
*  Copyright 2016 Mirco Nani
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*/

#pragma once

#include "ET_Interaction.h"
#include <eyex\EyeX.h>
#include <string.h>

class TobiiEyeX_GlobalGazeTracker
{
public:
	static TobiiEyeX_GlobalGazeTracker* Instance();
	void Init(ET_Producer<ET_GazeCoordinates_Content>* coordinatesSource, ET_Producer<ET_Log>* logSource = new ET_Producer<ET_Log>());
	unsigned int start( TX_GAZEPOINTDATAMODE filter = TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED );
	unsigned int stop();

protected:
		TobiiEyeX_GlobalGazeTracker();
		~TobiiEyeX_GlobalGazeTracker();

private:
		static TobiiEyeX_GlobalGazeTracker* instance; //=0
		static unsigned int running;
		static unsigned int ready;
};
