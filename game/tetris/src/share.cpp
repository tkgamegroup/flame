// MIT License
// 
// Copyright (c) 2019 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "share.h"

flame::graphics::Device *d;
flame::UI::Instance *ui;
flame::sound::Device *snd_d;
flame::sound::Context *snd_c;
flame::sound::Buffer *snd_buf_select;
flame::sound::Buffer *snd_buf_ok;
flame::sound::Buffer *snd_buf_back;
flame::sound::Buffer *snd_buf_mode;
flame::sound::Buffer *snd_buf_success;
flame::sound::Source *snd_src_select;
flame::sound::Source *snd_src_ok;
flame::sound::Source *snd_src_back;
flame::sound::Source *snd_src_mode;
flame::sound::Source *snd_src_success;
