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


#include "CaptionEntry.h"

const CaptionEntry CaptionEntry::Empty(0, 0, vector<Utf8String>());

CaptionEntry::CaptionEntry(int64_t startTime, int64_t endTime, vector< Utf8String > textLines)
{
	this->startTime = startTime;
	this->endTime = endTime;
	this->textLines = textLines;
}

CaptionEntry::~CaptionEntry(void)
{
}

CaptionEntry& CaptionEntry::operator=(CaptionEntry const& cpy)
{
	if(this != &cpy)
	{
		this->startTime = cpy.startTime;
		this->endTime = cpy.endTime;
		this->textLines = cpy.textLines;
	}

	return *this;
}

bool CaptionEntry::operator==(CaptionEntry const& cmp) const
{
	if(this->startTime != cmp.startTime || this->endTime != cmp.endTime || this->textLines.size() != cmp.textLines.size())
		return false;

	for(size_t i = 0; i < this->textLines.size(); ++i)
	{
		if(this->textLines[i] != cmp.textLines[i])
			return false;
	}

	return true;
}

Utf8String ReplaceNonPrintableChars(Utf8String const& str)
{
	Utf8String ret(str);

	size_t curOffset = ret.find('\231');

	while(curOffset != Utf8String::npos)
	{
		Utf8String tmp = ret.substr(0, curOffset);
		tmp.append("{bar quarter note}");
		tmp.append(ret.substr(curOffset + 1));
		ret = tmp;
		curOffset = ret.find('\231');
	}

	return ret;
}

#ifdef _WIN32
    const char* line = "\r\n";
#else
    const char* line = "\n";
#endif 
Utf8String CaptionEntry::toDisplayString() const
{
   
    Utf8String textLines;

    for(size_t i = 0; i < this->textLines.size(); ++i)
    {
        if(textLines.length() > 0)
            textLines.append(line);
        textLines.append(this->textLines.at(i));
    }
    
    return textLines;
}

Utf8String CaptionEntry::toDebugString() const
{
	Utf8String textLines;


	Utf8String text;

	text.Format
	(
		"Start: %lld End: %lld:  Text: %s ...",
		this->startTime, this->endTime,
		this->textLines.size() > 0 ? this->textLines[0].c_str() : "--None--"
	);

	return text;
}
