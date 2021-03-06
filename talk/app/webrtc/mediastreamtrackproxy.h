/*
 * libjingle
 * Copyright 2011, Google Inc.
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

// This file includes proxy classes for tracks. The purpose is
// to make sure tracks are only accessed from the signaling thread.

#ifndef TALK_APP_WEBRTC_MEDIASTREAMTRACKPROXY_H_
#define TALK_APP_WEBRTC_MEDIASTREAMTRACKPROXY_H_

#include <string>
#include <vector>

#include "talk/app/webrtc/audiotrack.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/videotrack.h"
#include "talk/base/thread.h"

namespace cricket {

class VideoCapturer;

}  // namespace cricket

namespace webrtc {

template <class T>
class MediaStreamTrackProxy : public T,
                              talk_base::MessageHandler {
 public:
  void Init(MediaStreamTrackInterface* track);
  // Implement MediaStreamTrack.

  virtual std::string kind() const;
  virtual std::string label() const;
  virtual bool enabled() const;
  virtual MediaStreamTrackInterface::TrackState state() const;
  virtual bool set_enabled(bool enable);
  virtual bool set_state(MediaStreamTrackInterface::TrackState new_state);

  // Implement Notifier
  virtual void RegisterObserver(ObserverInterface* observer);
  virtual void UnregisterObserver(ObserverInterface* observer);

 protected:
  explicit MediaStreamTrackProxy(talk_base::Thread* signaling_thread);

  void Send(uint32 id, talk_base::MessageData* data) const;
  // Returns true if the message is handled.
  bool HandleMessage(talk_base::Message* msg);

  mutable talk_base::Thread* signaling_thread_;
  MediaStreamTrackInterface* track_;
};

// AudioTrackProxy is a proxy for the AudioTrackInterface. The purpose is
// to make sure AudioTrack is only accessed from the signaling thread.
// It can be used as a proxy for both local and remote audio tracks.
class AudioTrackProxy : public MediaStreamTrackProxy<LocalAudioTrackInterface> {
 public:
  static talk_base::scoped_refptr<AudioTrackInterface> CreateRemote(
      const std::string& label,
      talk_base::Thread* signaling_thread);
  static talk_base::scoped_refptr<LocalAudioTrackInterface> CreateLocal(
      const std::string& label,
      AudioDeviceModule* audio_device,
      talk_base::Thread* signaling_thread);
  static talk_base::scoped_refptr<LocalAudioTrackInterface> CreateLocal(
      LocalAudioTrackInterface* implementation,
      talk_base::Thread* signaling_thread);

  virtual AudioDeviceModule* GetAudioDevice();

 protected:
  AudioTrackProxy(const std::string& label,
                  talk_base::Thread* signaling_thread);
  AudioTrackProxy(const std::string& label,
                  AudioDeviceModule* audio_device,
                  talk_base::Thread* signaling_thread);
  AudioTrackProxy(LocalAudioTrackInterface* implementation,
                  talk_base::Thread* signaling_thread);
  // Implement MessageHandler
  virtual void OnMessage(talk_base::Message* msg);

  talk_base::scoped_refptr<LocalAudioTrackInterface> audio_track_;
};

// VideoTrackProxy is a proxy for the VideoTrackInterface and
// LocalVideoTrackInterface. The purpose is
// to make sure VideoTrack is only accessed from the signaling thread.
// It can be used as a proxy for both local and remote video tracks.
class VideoTrackProxy : public MediaStreamTrackProxy<LocalVideoTrackInterface> {
 public:
  static talk_base::scoped_refptr<VideoTrackInterface> CreateRemote(
      const std::string& label,
      talk_base::Thread* signaling_thread);
  static talk_base::scoped_refptr<LocalVideoTrackInterface> CreateLocal(
      const std::string& label,
      cricket::VideoCapturer* video_device,
      talk_base::Thread* signaling_thread);
  static talk_base::scoped_refptr<LocalVideoTrackInterface> CreateLocal(
      LocalVideoTrackInterface* implementation,
      talk_base::Thread* signaling_thread);

  virtual cricket::VideoCapturer* GetVideoCapture();

  virtual void AddRenderer(VideoRendererInterface* renderer);
  virtual void RemoveRenderer(VideoRendererInterface* renderer);
  virtual cricket::VideoRenderer* FrameInput();

 protected:
  VideoTrackProxy(const std::string& label,
                  talk_base::Thread* signaling_thread);
  VideoTrackProxy(const std::string& label,
                  cricket::VideoCapturer* video_device,
                  talk_base::Thread* signaling_thread);
  VideoTrackProxy(LocalVideoTrackInterface* implementation,
                  talk_base::Thread* signaling_thread);

  // Implement MessageHandler
  virtual void OnMessage(talk_base::Message* msg);

  talk_base::scoped_refptr<LocalVideoTrackInterface> video_track_;
};

}  // namespace webrtc

#endif  // TALK_APP_WEBRTC_MEDIASTREAMTRACKPROXY_H_
