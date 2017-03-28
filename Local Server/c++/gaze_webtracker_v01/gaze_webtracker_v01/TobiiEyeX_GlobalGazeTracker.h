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
