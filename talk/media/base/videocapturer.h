// libjingle
// Copyright 2010 Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. The name of the author may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Declaration of abstract class VideoCapturer

#ifndef TALK_MEDIA_BASE_VIDEOCAPTURER_H_
#define TALK_MEDIA_BASE_VIDEOCAPTURER_H_

#include <string>
#include <vector>

#include "talk/base/basictypes.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/sigslot.h"
#include "talk/media/base/videocommon.h"
#include "talk/media/devices/devicemanager.h"

namespace cricket {

// Current state of the capturer.
// TODO(hellner): CS_NO_DEVICE is an error code not a capture state. Separate
//                error codes and states.
enum CaptureState {
  CS_STOPPED,    // The capturer has been stopped or hasn't started yet.
  CS_STARTING,   // The capturer is in the process of starting. Note, it may
                 // still fail to start.
  CS_RUNNING,    // The capturer has been started successfully and is now
                 // capturing.
  CS_PAUSED,     // The capturer has been paused.
  CS_FAILED,     // The capturer failed to start.
  CS_NO_DEVICE,  // The capturer has no device and consequently failed to start.
};

class VideoFrame;

struct CapturedFrame {
  static const uint32 kFrameHeaderSize = 40;  // Size from width to data_size.
  static const uint32 kUnknownDataSize = 0xFFFFFFFF;

  CapturedFrame();

  // Get the number of bytes of the frame data. If data_size is known, return
  // it directly. Otherwise, calculate the size based on width, height, and
  // fourcc. Return true if succeeded.
  bool GetDataSize(uint32* size) const;

  // The width and height of the captured frame could be different from those
  // of VideoFormat. Once the first frame is captured, the width, height,
  // fourcc, pixel_width, and pixel_height should keep the same over frames.
  int    width;         // in number of pixels
  int    height;        // in number of pixels
  uint32 fourcc;        // compression
  uint32 pixel_width;   // width of a pixel, default is 1
  uint32 pixel_height;  // height of a pixel, default is 1
  int64  elapsed_time;  // elapsed time since the creation of the frame
                        // source (that is, the camera), in nanoseconds.
  int64  time_stamp;    // timestamp of when the frame was captured, in
                        // nanoseconds.
  uint32 data_size;     // number of bytes of the frame data
  int    rotation;      // rotation in degrees of the frame (0, 90, 180, 270)
  void*  data;          // pointer to the frame data. This object allocates the
                        // memory or points to an existing memory.

 private:
  DISALLOW_COPY_AND_ASSIGN(CapturedFrame);
};

// VideoCapturer is an abstract class that defines the interfaces for video
// capturing. The subclasses implement the video capturer for various platforms.
//
// The captured frames may need to be adapted (for example, cropping). Such an
// adaptor is out of the scope of VideoCapturer. If the adaptor is needed, it
// acts as the downstream of VideoCapturer, adapts the captured frames, and
// delivers the adapted frames to other components such as the encoder.
//
// Programming model:
//   Create and initialize an object of a subclass of VideoCapturer
//   SignalStateChange.connect()
//   SignalFrameCaptured.connect()
//   Find the capture format for Start() by either calling GetSupportedFormats()
//   and selecting one of the supported or calling GetBestCaptureFormat().
//   Start()
//   GetCaptureFormat() optionally
//   Stop()
//
// Assumption:
//   The Start() and Stop() methods are called by a single thread (that is, the
//   media engine thread). Hence, there is no need to make them thread safe.
//
class VideoCapturer : public sigslot::has_slots<> {
 public:
  VideoCapturer();
  virtual ~VideoCapturer() {}

  // Gets the id of the underlying device, which is available after the capturer
  // is initialized. Can be used to determine if two capturers reference the
  // same device.
  const std::string& GetId() const { return id_; }

  // Get the capture formats supported by the video capturer. The supported
  // formats are available after the device is opened successfully.
  // Return NULL if the supported formats are not available.
  const std::vector<VideoFormat>* GetSupportedFormats() const {
    return supported_formats_.get();
  }

  // Get the best capture format for the desired format. The best format is the
  // same as one of the supported formats except that the frame interval may be
  // different. If the application asks for 16x9 and the camera does not support
  // 16x9 HD or the application asks for 16x10, we find the closest 4x3 and then
  // crop; Otherwise, we find what the application asks for. Note that we assume
  // that for HD, the desired format is always 16x9. The subclasses can override
  // the default implementation.
  // Parameters
  //   desired: the input desired format. If desired.fourcc is not kAnyFourcc,
  //            the best capture format has the exactly same fourcc. Otherwise,
  //            the best capture format uses a fourcc in GetPreferredFourccs().
  //   best_format: the output of the best capture format.
  // Return false if there is no such a best format, that is, the desired format
  // is not supported.
  virtual bool GetBestCaptureFormat(const VideoFormat& desired,
                                    VideoFormat* best_format);

  // Start the video capturer with the specified capture format.
  // Parameter
  //   capture_format: The caller got this parameter by either calling
  //                   GetSupportedFormats() and selecting one of the supported
  //                   or calling GetBestCaptureFormat().
  // Return
  //   CS_STARTING:  The capturer is trying to start. Success or failure will
  //                 be notified via the |SignalStateChange| callback.
  //   CS_RUNNING:   if the capturer is started and capturing.
  //   CS_PAUSED:    Will never be returned.
  //   CS_FAILED:    if the capturer failes to start..
  //   CS_NO_DEVICE: if the capturer has no device and fails to start.
  virtual CaptureState Start(const VideoFormat& capture_format) = 0;

  // Get the current capture format, which is set by the Start() call.
  // Note that the width and height of the captured frames may differ from the
  // capture format. For example, the capture format is HD but the captured
  // frames may be smaller than HD.
  const VideoFormat* GetCaptureFormat() const {
    return capture_format_.get();
  }

  // Stop the video capturer.
  virtual void Stop() = 0;
  // Check if the video capturer is running.
  virtual bool IsRunning() = 0;

  // Signal all capture state changes that are not a direct result of calling
  // Start().
  sigslot::signal2<VideoCapturer*, CaptureState> SignalStateChange;
  // TODO(hellner): rename |SignalFrameCaptured| to something like
  //                |SignalRawFrame| or |SignalNativeFrame|.
  // Signal the captured frame to downstream.
  sigslot::signal2<VideoCapturer*, const CapturedFrame*> SignalFrameCaptured;
  // Signal the captured frame converted to I420 to downstream.
  sigslot::signal2<VideoCapturer*, const VideoFrame*> SignalVideoFrame;
  // Signals a change in capturer state.

 protected:
  // Callback attached to SignalFrameCaptured where SignalVideoFrames is called.
  void OnFrameCaptured(VideoCapturer*, const CapturedFrame* captured_frame);

  // subclasses override this virtual method to provide a vector of fourccs, in
  // order of preference, that are expected by the media engine.
  virtual bool GetPreferredFourccs(std::vector<uint32>* fourccs) = 0;

  // mutators to set private attributes
  void SetId(const std::string& id) {
    id_ = id;
  }

  void SetCaptureFormat(const VideoFormat* format) {
    capture_format_.reset(format ? new VideoFormat(*format) : NULL);
  }

  void SetSupportedFormats(const std::vector<VideoFormat>& formats);

 private:
  // Get the distance between the desired format and the supported format.
  // Return the max distance if they mismatch. See the implementation for
  // details.
  int64 GetFormatDistance(const VideoFormat& desired,
                          const VideoFormat& supported);

  std::string id_;
  talk_base::scoped_ptr<VideoFormat> capture_format_;
  talk_base::scoped_ptr<std::vector<VideoFormat> > supported_formats_;

  DISALLOW_COPY_AND_ASSIGN(VideoCapturer);
};

}  // namespace cricket

#endif  // TALK_MEDIA_BASE_VIDEOCAPTURER_H_
