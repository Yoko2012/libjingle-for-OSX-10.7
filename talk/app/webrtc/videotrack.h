/*
 * libjingle
 * Copyright 2012, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TALK_APP_WEBRTC_VIDEOTRACK_H_
#define TALK_APP_WEBRTC_VIDEOTRACK_H_

#include <string>

#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/mediastreamtrack.h"
#include "talk/app/webrtc/notifier.h"
#include "talk/app/webrtc/videotrackrenderers.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/scoped_ref_ptr.h"

#ifdef WEBRTC_RELATIVE_PATH
#include "modules/video_capture/main/interface/video_capture.h"
#else
#include "third_party/webrtc/modules/video_capture/main/interface/video_capture.h"
#endif

namespace cricket {

class VideoCapturer;

}  // namespace cricket

namespace webrtc {

class VideoTrack : public MediaStreamTrack<LocalVideoTrackInterface> {
 public:
  // Create a video track used for remote video tracks.
  static talk_base::scoped_refptr<VideoTrack> CreateRemote(
      const std::string& label);
  // Create a video track used for local video tracks.
  static talk_base::scoped_refptr<VideoTrack> CreateLocal(
      const std::string& label,
      cricket::VideoCapturer* video_device);

  virtual cricket::VideoCapturer* GetVideoCapture();

  virtual void AddRenderer(VideoRendererInterface* renderer);
  virtual void RemoveRenderer(VideoRendererInterface* renderer);
  virtual cricket::VideoRenderer* FrameInput();

  virtual std::string kind() const;

 protected:
  explicit VideoTrack(const std::string& label);
  VideoTrack(const std::string& label, cricket::VideoCapturer* video_device);

 private:
  VideoTrackRenderers renderers_;
  talk_base::scoped_ptr<cricket::VideoCapturer> video_device_;
};

}  // namespace webrtc

#endif  // TALK_APP_WEBRTC_VIDEOTRACK_H_
