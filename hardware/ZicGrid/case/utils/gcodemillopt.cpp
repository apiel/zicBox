// gcodemillopt.cpp
// Copyright 2014 - Andrew L. Sandoval
// Optimizes gcode from MakerCam for CNC mills and CNC laser engravers/burners
// Also modifies MakerCAM CNC mill gcodes to laser codes WITHOUT depth (laser on/off before moves)
//
// Copyright(c) 2014, Andrew L. Sandoval
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and / or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// 

#include <stdio.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <array>
#include <algorithm>
#include <memory>
#include <utility>
#include <sys/stat.h>

class GCodeSet
{
private:
	double m_dXStart;
	double m_dYStart;
	double m_dZStart;
	double m_dXEnd;
	double m_dYEnd;
	double m_dZEnd;
	std::vector<std::string> m_vLines;
public:
	GCodeSet(double dX = 0.0f, double dY = 0.0f, double dZ = 0.0f) : 
		m_dXStart(dX), m_dYStart(dY), m_dZStart(dZ),
		m_dXEnd(0.0f), m_dYEnd(0.0f), m_dZEnd(0.0f)
	{
	}

	GCodeSet(const GCodeSet &copy) : m_dXStart(copy.m_dXStart),
		m_dYStart(copy.m_dYStart), m_dZStart(copy.m_dZStart),
		m_dXEnd(copy.m_dXEnd), m_dYEnd(copy.m_dYEnd),
		m_dZEnd(copy.m_dZEnd)
	{
		std::copy(copy.GetLines().begin(), copy.GetLines().end(), 
			std::back_inserter(m_vLines));
	}

	void operator=(const GCodeSet &copy)
	{
		m_dXStart = copy.m_dXStart;
		m_dYStart = copy.m_dYStart;
		m_dZStart = copy.m_dZStart;
		m_dXEnd = copy.m_dXEnd;
		m_dYEnd = copy.m_dYEnd;
		m_dZEnd = copy.m_dZEnd;
		m_vLines.clear();
		std::copy(copy.GetLines().begin(), copy.GetLines().end(), 
			std::back_inserter(m_vLines));
	}

	void Reset(double dXStart = 0.0f, double dYStart = 0.0f, double dZStart = 0.0f)
	{
		m_dXStart = dXStart;
		m_dYStart = dYStart;
		m_dZStart = dZStart;
		m_dXEnd = 0.0f;
		m_dYEnd = 0.0f;
		m_dZEnd = 0.0f;
		m_vLines.clear();
	}

	double GetXStart() const
	{
		return m_dXStart;
	}

	double GetYStart() const
	{
		return m_dYStart;
	}

	double GetZStart() const
	{
		return m_dZStart;
	}

	double GetXEnd() const
	{
		return m_dXEnd;
	}

	double GetYEnd() const
	{
		return m_dYEnd;
	}

	double GetZEnd() const
	{
		return m_dZEnd;
	}

	void SetXEnd(double dXEnd)
	{
		m_dXEnd = dXEnd;
	}

	void SetYEnd(double dYEnd)
	{
		m_dYEnd = dYEnd;
	}

	void SetZEnd(double dZEnd)
	{
		m_dZEnd = dZEnd;
	}

	void SetZStart(double dZStart)
	{
		m_dZStart = dZStart;
	}

	const std::vector<std::string>& GetLines() const
	{
		return m_vLines;
	}

	std::vector<std::string>& LineVector()
	{
		return m_vLines;
	}

};

//
// RAII helper to only update a previous line on loop/scope end
class PreviousLine
{
private:
	std::string &m_rstrPreviousLine;
	const std::string &m_rstrCurrentLine;
public:
	PreviousLine(std::string &rstrPreviousLine, const std::string &rstrCurrentLine) :
		m_rstrPreviousLine(rstrPreviousLine), m_rstrCurrentLine(rstrCurrentLine)
	{
	}

	~PreviousLine()
	{
		m_rstrPreviousLine.assign(m_rstrCurrentLine.c_str());
	}
};

//
// Sorting...
bool SortLessPair(std::pair<std::vector<GCodeSet>::size_type, double> &left, 
	std::pair<std::vector<GCodeSet>::size_type, double> &right) 
{
	return left.second < right.second;
}

bool FileExists(const std::string &strFilename)
{
	struct stat st = { };
	if (stat(strFilename.c_str(), &st) == -1)
	{
		fprintf(stderr, "Error %i opening %hs\n", errno, strFilename.c_str());
		return false;
	}
	return true;
}

int fclose_checked(FILE *pfFile)
{
	if (pfFile != stdout && pfFile != nullptr)
	{
		return fclose(pfFile);
	}
	return 0;
}

void OptimizeAndOutputSets(std::deque<GCodeSet> &vSets, unsigned int &uiLinesOut, FILE *pfOutput = stdout)
{
	double dDepth = 0.0f;
	if (vSets.empty() == false)
	{
		dDepth = vSets[0].GetZStart();
	}
	fprintf(stderr, "Sorting %u sets at depth %.02f\n", vSets.size(), dDepth);

	//
	// 
	// SLOW sort algorithm, but the idea is to find the nearest place to jump to after completing
	// one operation (set).  So we calculate the absolute nearest over and over as we move.  The result
	// is very few long jumps (linear G0)!
	double dXLast = 0.0f;
	double dYLast = 0.0f;
	std::deque<GCodeSet> vOptimizedSet;
	while (vSets.empty() == false)
	{
		//
		// Search for nearest to dXLast, dYLast
		std::deque<std::pair<std::vector<GCodeSet>::size_type, double> > vDistance;
		std::deque<GCodeSet>::size_type index = 0;
		for (auto entry : vSets)
		{
			double dXDelta = entry.GetXStart() - dXLast;
			double dYDelta = entry.GetYStart() - dYLast;
			vDistance.push_back(std::make_pair(index, abs(dXDelta) + abs(dYDelta)));
			++index;
		}
		std::sort(vDistance.begin(), vDistance.end(), SortLessPair);
		std::deque<GCodeSet>::size_type szIndex = vDistance.front().first;
		vOptimizedSet.push_back(vSets[szIndex]);
		auto erasey = vSets.begin() + szIndex;
		vSets.erase(erasey);
		dXLast = vOptimizedSet.back().GetXEnd();
		dYLast = vOptimizedSet.back().GetYEnd();
	}

	for (auto entry : vOptimizedSet)
	{
		fprintf(stderr, "\tOutputing set with %u lines\n", entry.GetLines().size());
		for (auto line : entry.GetLines())
		{
			++uiLinesOut;
			fprintf(pfOutput, line.c_str());
		}
	}
}

int main(int argc, const char *ppArgv[])
{
	std::shared_ptr<FILE> spfOutput(stdout, fclose_checked);
	const char *pszInputFile = nullptr;
	bool bLaserMode = false;
	unsigned int uiLinesOut = 0;

	//
	// Parse command-line arguments:
	// -o outputfile
	// -i inputfile
	// -l or -laser = laser-mode
	std::string strAppName(ppArgv[0]);
#ifdef _WIN32
	std::string::size_type szSlash = strAppName.rfind('\\');
#else
	std::string::size_type szSlash = strAppName.rfind('/');
#endif
	if (szSlash != strAppName.npos)
	{
		strAppName.erase(0, szSlash);
	}

	//
	// Turn on laser mode if the executable name contains the word laser, e.g. gcodeoptlaser
	std::transform(strAppName.begin(), strAppName.end(), strAppName.begin(), ::tolower);
	if (strAppName.npos != strAppName.find("laser"))
	{
		bLaserMode = true;
	}

	for (int iArg = 1; iArg < argc; ++iArg)
	{
		const std::string strOption(ppArgv[iArg]);
		if (strOption == "-o" && iArg < (argc - 1) && spfOutput.get() == stdout)
		{
			spfOutput.reset(fopen(ppArgv[iArg + 1], "wt"), fclose);
			if (nullptr == spfOutput)
			{
				fprintf(stderr, "Error %i opening %hs\n", errno, ppArgv[iArg + 1]);
				return 0;
			}
			++iArg;
		}
		else if (strOption == "-laser" || strOption == "-l")
		{
			bLaserMode = true;
		}
		else if (strOption == "-i" && pszInputFile == nullptr && iArg < (argc - 1))
		{
			if (false == FileExists(ppArgv[iArg + 1]))
			{
				return 0;
			}
			pszInputFile = ppArgv[iArg + 1];
			++iArg;
		}
		else if (pszInputFile == nullptr &&
			FileExists(strOption))
		{
			pszInputFile = ppArgv[iArg];
		}
		else
		{
			fprintf(stderr, "Unknown command-line argument: %hs\n", strOption.c_str());
			return 0;
		}
	}

	if (pszInputFile == nullptr)
	{
		fprintf(stderr, "USAGE: %hs inputfile [-o outputfile][-laser|-l][-x|-y|-xy]\n", ppArgv[0]);
		return 0;
	}

	std::shared_ptr<FILE> spFile(fopen(ppArgv[1], "rt"), fclose);
	if (spFile.get() == nullptr)
	{
		fprintf(stderr, "Error %i opening %hs\n", errno, ppArgv[1]);
		return 1;
	}

	unsigned int uiLine = 0;
	std::deque<GCodeSet> vCurrentSets;

	bool bInPrologue = true;
	bool bInEpilogue = false;
	GCodeSet gcCurrent(0.0f, 0.0f, 0.0f);
	std::string strPreviousLine;
	double dCurrentZ = 0.0f;
	double dCurrentX = 0.0f;
	double dCurrentY = 0.0f;
	double dLastZ = 0.0f;
	double dLastX = 0.0f;
	double dLastY = 0.0f;
	unsigned int uiSets = 0;
	bool bCutterEngaged = false;

	//
	// Add an attribution
	fprintf(spfOutput.get(), "(Optimized by %hs)\n(Written by Andrew L. Sandoval)\n", strAppName.c_str());

	// Read file and break into sections...
	while (!feof(spFile.get()))
	{
		std::array<char, 2048> szLine = { };
		if (nullptr == fgets(&szLine[0], szLine.size(), spFile.get()))
		{
			continue;
		}
		++uiLine;
		std::string strLine(&szLine[0]);
		std::string strLineCodes(strLine);
		std::string::size_type szLE = strLineCodes.find_first_of("\r\n");
		if (szLE != strLineCodes.npos)
		{
			strLineCodes.erase(szLE);
		}
		PreviousLine prevLine(strPreviousLine, strLine);
		std::string::size_type szX = strLine.find(" X");
		std::string::size_type szY = strLine.find(" Y");
		std::string::size_type szZ = strLine.find(" Z");
		if (strLine.length() > 1 && strLine[0] == 'G' && strLine[1] == '0')
		{
			bCutterEngaged = false;
		}
		else if (strLine.length() > 2 && strLine[0] == 'G' && strLine[1] == '1' &&
			false == isdigit(strLine[2])) // Don't clobber G17
		{
			bCutterEngaged = true;
		}
		else if (bInPrologue)
		{
			if (strLineCodes == "M3" && bLaserMode)
			{
				fprintf(spfOutput.get(), "(M3 - removed in laser mode)\n");
			}
			else
			{
				fprintf(spfOutput.get(), strLine.c_str());
			}
			++uiLinesOut;
			continue;	// Stay in prologue until a G0/G1
		}

		//
		// From here on, past prologue

		//
		// Check for  start of epilogue (before we much with M5's)
		if (strLine.find("M5") == 0 ||
			strLine.find("M30") == 0)
		{
			// sort and flush last set, in epilogue -- sort and output vCurrentSets
			vCurrentSets.push_back(gcCurrent);
			OptimizeAndOutputSets(vCurrentSets, uiLinesOut, spfOutput.get());
			gcCurrent.Reset();
			vCurrentSets.clear();
			// Only supporting one tool currently, so first spindle stop means prologue...
			bInEpilogue = true;
			bInPrologue = false;
			if (dCurrentZ < 0.0f && bLaserMode == false)
			{
				fprintf(stderr, "WARNING!!!! Program Ends with cutter down, Z = %.02f\n", dCurrentZ);
			}
		}

		if (bInEpilogue)
		{
			fprintf(spfOutput.get(), strLine.c_str());
			++uiLinesOut;
			continue;
		}

		//
		// G0 w/Z or G1 w/Z
		// If Laser Mode, change any G0 Z to an M5 (laser off)
		// If Laser Mode, change and G1 Z to M3 (laser on)
		if (strLine[0] == 'G' && szZ != strLine.npos)
		{
			if (bCutterEngaged)
			{
				dLastZ = dCurrentZ;
				dCurrentZ = atof(strLine.c_str() + szZ + 2);
				gcCurrent.SetZStart(dCurrentZ);

				if (dLastZ != dCurrentZ)
				{
					// sort and flush last set, new depth -- sort and output vCurrentSets
					// This line will end up in gcCurrent so it can be ignored
					OptimizeAndOutputSets(vCurrentSets, uiLinesOut, spfOutput.get());
					vCurrentSets.clear();
				}
			}

			if (bLaserMode)
			{
				// G0 Z (replace with M3 - shut off laser)
				// G1 Z (replace with M5)
				if (bCutterEngaged == false)
				{
					// Moving with laser disengaged
					strLine = "M5 (laser off - was ";
					strLine += strLineCodes;
					strLine += ")\n";
				}
				else
				{
					strLine = "M3 (laser on - was ";
					strLine += strLineCodes;
					strLine += ")\n";
				}
				if (bInPrologue)		// Only place this should happen...
				{
					fprintf(spfOutput.get(), strLine.c_str());
				}
			}
		}

		//
		// G0 w/X or G1 w/X
		if (strLine[0] == 'G' &&		// Cutter engaged or not, move to X
			szX != strLine.npos)
		{
			// Update current position for tracking end position of set
			dLastX = dCurrentX;
			dCurrentX = atof(strLine.c_str() + szX + 2);
		}

		//
		// G0 w/Y or G1 w/Y
		if (strLine[0] == 'G' &&		// Cutter engaged or not, move to Y
			szY != strLine.npos)
		{
			// Update current position for tracking end position of set
			dLastY = dCurrentY;
			dCurrentY = atof(strLine.c_str() + szY + 2);
		}

		//
		// G0 X - start of a set
		if (strLine.find("G0 X") == 0)	// Rapid Linear Motion, cutter not engaged
		{
			bInPrologue = false;
			bInEpilogue = false;
			// NOTE: Though x and y will be the end of the G0 rapid linear motion, it is the start of a set
			// where cutting begins!
			
			// Error if the depth is not positive!!!

			// Start of a new set...
			if (szX == strLine.npos ||
				szY == strLine.npos)
			{
				fprintf(stderr, "Critical Error: expected a G0 line to contain X and Y axis settings on line #%u: %hs\n", uiLine, strLine.c_str());
				return 0;
			}

			//
			// Update the last set's end point:
			gcCurrent.SetZEnd(dLastZ);
			gcCurrent.SetXEnd(dLastX);
			gcCurrent.SetYEnd(dLastY);

			if (gcCurrent.GetLines().empty() == false)
			{
				vCurrentSets.push_back(gcCurrent);
			}
			// A new set is started with a G0 X
			// When a G1 Z to a new depth is encountered all previous sets can be sorted and output
			// Same for epilogue start
			//
			// Record new set's start point and update references
			gcCurrent.Reset(dCurrentX, dCurrentY, dCurrentZ);	 // Current Z may change on next line
			++uiSets;
		}
		//
		// Part of a set...
		if (bInPrologue == true)
		{
			continue;
		}
		gcCurrent.LineVector().push_back(strLine);
	}

	fprintf(stderr, "Output Lines: %lu\n", uiLinesOut);
	fprintf(stderr, "Input Lines: %lu\n", uiLine);
	return 0;
}

