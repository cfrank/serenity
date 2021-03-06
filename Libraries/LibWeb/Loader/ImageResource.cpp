/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <AK/Function.h>
#include <LibGfx/Bitmap.h>
#include <LibGfx/ImageDecoder.h>
#include <LibWeb/Loader/ImageResource.h>

namespace Web {

ImageResource::ImageResource(const LoadRequest& request)
    : Resource(Type::Image, request)
{
}

ImageResource::~ImageResource()
{
}

Gfx::ImageDecoder& ImageResource::ensure_decoder()
{
    if (!m_decoder)
        m_decoder = Gfx::ImageDecoder::create(encoded_data());
    return *m_decoder;
}

void ImageResource::update_volatility()
{
    if (!m_decoder)
        return;

    bool visible_in_viewport = false;
    for_each_client([&](auto& client) {
        if (static_cast<const ImageResourceClient&>(client).is_visible_in_viewport())
            visible_in_viewport = true;
    });

    if (!visible_in_viewport) {
        m_decoder->set_volatile();
        return;
    }

    bool still_has_decoded_image = m_decoder->set_nonvolatile();
    if (still_has_decoded_image)
        return;

    m_decoder = nullptr;
    for_each_client([&](auto& client) {
        static_cast<ImageResourceClient&>(client).resource_did_replace_decoder();
    });
}

ImageResourceClient::~ImageResourceClient()
{
}

}
