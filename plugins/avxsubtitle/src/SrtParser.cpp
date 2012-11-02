// Copyright (c) 2012 David Ronca.  All rights reserved.
// videophool@hotmail.com
// http://www.avxsynth.org


// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.auto


#include <stdio.h>
#include <locale.h>
#include <wctype.h>
#include "Utils.h"
#include "SrtParser.h"
#include "AvxException.h"

class _SrtParserWorker
{
public:

    static long long ParseTime(Utf8String const& timeStr)
    {
        Utf8String parseStr(timeStr);

        Utf8String hours = parseStr.Parse(":");
        Utf8String minutes = parseStr.Parse(":");
        Utf8String seconds = parseStr.Parse(",");
        Utf8String milliSecs = parseStr;

        long long time = strtol(hours, 0, 10) * 3600;
        time += strtol(minutes, 0, 10) * 60;
        time += strtol(seconds, 0, 10);
        time *= 1000;
        time += strtol(milliSecs, 0, 10);
        time *= 10000L;
        return time;
    }

    static void ParseTimeInfo(Utf8String const& line, long long& startTime, long long& endTime)
    {
        size_t sep = line.find("-->");

        if(sep == Utf8String::npos)
            AvxException::Throw("Invalid time format: %s", line.c_str());

        startTime = ParseTime(Utf8String(line.substr(0, sep - 1)));
        endTime = ParseTime(line.substr(sep + 3));
    }

    static CaptionEntry ReadNextEntry(CaptionReader & reader)
    {
        // skip blank lines

        Utf8String curLine;

        curLine = reader.ReadLine();
        while(curLine.Trim().length() == 0)
            curLine = reader.ReadLine();

        // expect caption #
        for(size_t i = 0; i < curLine.length(); ++i)
        {
            if(!(iswdigit(curLine.at(i))))
            {
                const char* p = curLine.c_str();
                AvxException::Throw("Invalid SRT. Expected caption number.  Was: %hs", curLine.c_str());
            }
        }

        long long startTime;
        long long endTime;
        vector<Utf8String> text;

        try
        {
            Utf8String captionNum = curLine;

            // verify captionNum

            Utf8String timeInfo = reader.ReadLine().Trim();

            ParseTimeInfo(timeInfo, startTime, endTime);


            // now every line is text until empty line
            curLine = reader.ReadLine();
            while(curLine.Trim().length() != 0)
            {
                text.push_back(curLine.Trim());
                curLine = reader.ReadLine();
            }

        }
        catch(CaptionReader::EOFException &)
        {
            //AvxException::Throw("Unexpected EOF while parsing caption");
        }

        CaptionEntry ret(startTime, endTime, text);
        return ret;
    }

};

SRTParser::SRTParser(const char* path) : entryCount(0)
{
    try
    {
        if(!IsSRT(path))
            AvxException::Throw("File is not SRT file: %s", path);

        CaptionReader reader(path);

        CaptionEntry curEntry = _SrtParserWorker::ReadNextEntry(reader);

        while(1)
        {
            captions.push_back(curEntry);
            ++entryCount;
            curEntry = _SrtParserWorker::ReadNextEntry(reader);
        }
    }
    catch(CaptionReader::EOFException &)
    {
    }
}

bool SRTParser::IsSRT(const char* path)
{
    CaptionReader reader(path);

    try
    {
        CaptionEntry entry = _SrtParserWorker::ReadNextEntry(reader);

        return true;
    }
    catch(AvxException&){}

    return false;
}


CaptionList SRTParser::Parse(const char* path)
{
    SRTParser parser(path);
    return parser.captions;
}





