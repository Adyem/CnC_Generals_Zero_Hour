/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "streamer.h"

Streamer::Streamer() : Output_Device(nullptr), Buffer()
{
    Buffer.reserve(STREAMER_BUFSIZ);
}

Streamer::~Streamer()
{
    sync();
}

int Streamer::setOutputDevice(OutputDevice *device)
{
    Output_Device = device;
    return 0;
}

std::streamsize Streamer::xsputn(const char_type* buf, std::streamsize size)
{
    if (size <= 0) {
        return 0;
    }

    std::streamsize written = 0;
    for (std::streamsize i = 0; i < size; ++i) {
        if (traits_type::eq_int_type(overflow(traits_type::to_int_type(buf[i])), traits_type::eof())) {
            break;
        }
        ++written;
    }

    return written;
}

Streamer::int_type Streamer::overflow(int_type ch)
{
    if (traits_type::eq_int_type(ch, traits_type::eof())) {
        return sync() == 0 ? traits_type::not_eof(ch) : traits_type::eof();
    }

    Buffer.push_back(static_cast<char>(traits_type::to_char_type(ch)));

    if (Buffer.size() >= STREAMER_BUFSIZ || traits_type::to_char_type(ch) == '\n') {
        flushBuffer();
    }

    return ch;
}

int Streamer::sync()
{
    flushBuffer();
    return 0;
}

void Streamer::flushBuffer()
{
    if (Output_Device && !Buffer.empty()) {
        Output_Device->print(Buffer.data(), static_cast<int>(Buffer.size()));
    }

    Buffer.clear();
}
