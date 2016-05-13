// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnBufferingUpdateListener;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnInfoListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.media.MediaPlayer.OnSeekCompleteListener;
import android.media.MediaPlayer.OnTimedMetaDataAvailableListener;
import android.media.MediaPlayer.OnTimedTextListener;
import android.media.MediaPlayer.OnVideoSizeChangedListener;
import android.media.MediaPlayer.TrackInfo;
import android.media.MediaTimestamp;
import android.media.PlaybackParams;
import android.media.SyncParams;
import android.net.Uri;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.io.FileDescriptor;
import java.util.HashMap;
import java.util.Map;

@XWalkAPI(createExternally = true)
public class XWalkMediaPlayerInternal extends MediaPlayer {
    private final static String TAG = "XWalkMediaPlayerInternal";

    private void unsupported() {
        Log.e(TAG, "ERROR: The function must be implemented");
        throw new UnsupportedOperationException();
    }

    /**
     * Sets the Surface to be used as the sink for the video portion of the media.
     * @param surface the Surface to be used for the video portion of the media.
     * @since 7.0
     */
    @XWalkAPI
    public void setSurface(Surface surface) {
        unsupported();
    }

    /**
     * Sets the data source as a content Uri.
     * @param context the Context to use when resolving the Uri.
     * @param uri the Content URI of the data you want to play.
     * @param headers the headers to be sent together with the request for the data.
     * @since 7.0
     */
    @XWalkAPI
    public void setDataSource(Context context, Uri uri, Map<String, String> headers) {
        unsupported();
    }

    /**
     * Sets the data source (FileDescriptor) to use.
     * @param fd the FileDescriptor for the file you want to play.
     * @param offset the offset into the file where the data to be played starts, in bytes.
     * @param length the length in bytes of the data to be played.
     * @since 7.0
     */
    @XWalkAPI
    public void setDataSource(FileDescriptor fd, long offset, long length) {
        unsupported();
    }

    /**
     * Sets the data source as a content Uri.
     * @param context the Context to use when resolving the Uri.
     * @param uri the Content URI of the data you want to play.
     * @since 7.0
     */
    @XWalkAPI
    public void setDataSource(Context context, Uri uri) {
        unsupported();
    }

    /**
     * Prepares the player for playback, asynchronously.
     * @since 7.0
     */
    @XWalkAPI
    public void prepareAsync() {
        unsupported();
    }

    /**
     * Checks whether the MediaPlayer is playing.
     * @return true if currently playing, false otherwise.
     * @since 7.0
     */
    @XWalkAPI
    public boolean isPlaying() {
        // unsupported();
        return false;
    }

    /**
     * Returns the width of the video.
     * @return the width of the video, or 0 if there is no video.
     * @since 7.0
     */
    @XWalkAPI
    public int getVideoWidth() {
        unsupported();
        return 0;
    }

    /**
     * Returns the height of the video.
     * @return the height of the video, or 0 if there is no video.
     * @since 7.0
     */
    @XWalkAPI
    public int getVideoHeight() {
        unsupported();
        return 0;
    }

    /**
     * Gets the current playback position.
     * @return the current position in milliseconds.
     * @since 7.0
     */
    @XWalkAPI
    public int getCurrentPosition() {
        // unsupported();
        return 0;
    }

    /**
     * Gets the duration of the file.
     * @return the duration in milliseconds, if no duration is
     * available (for example, if streaming live content), -1 is returned.
     * @since 7.0
     */
    @XWalkAPI
    public int getDuration() {
        unsupported();
        return 0;
    }

    /**
     * Releases resources associated with this MediaPlayer object.
     * @since 7.0
     */
    @XWalkAPI
    public void release() {
        unsupported();
    }

    /**
     * Sets the volume on this player.
     * @param volume1 left volume scalar.
     * @param volume2 right volume scalar.
     * @since 7.0
     */
    @XWalkAPI
    public void setVolume(float volume1, float volume2) {
        unsupported();
    }

    /**
     * Starts or resumes playback. If playback had previously been paused,
     * playback will continue from where it was paused. If playback had been stopped,
     * or never started before, playback will start at the beginning.
     * @since 7.0
     */
    @XWalkAPI
    public void start() {
        unsupported();
    }

    /**
     * Pauses playback. Call {@link XWalkMediaPlayerInternal#start()} to resume.
     * @since 7.0
     */
    @XWalkAPI
    public void pause() {
        unsupported();
    }

    /**
     * Seeks to specified time position.
     * @param msec the offset in milliseconds from the start to seek to.
     * @since 7.0
     */
    @XWalkAPI
    public void seekTo(int msec) {
        unsupported();
    }

    /**
     * Returns an array of track information.
     * @return array of track info.
     * @since 7.0
     */
    @XWalkAPI
    public TrackInfo[] getTrackInfo() {
        unsupported();
        return null;
    }

    /**
     * Register a callback to be invoked when the status of a network stream's
     * buffer has changed.
     * @param listener the callback that will be run.
     * @since 7.0
     */
    @XWalkAPI
    public void setOnBufferingUpdateListener(OnBufferingUpdateListener listener) {
        unsupported();
    }

    /**
     * Register a callback to be invoked when the end of a media source has
     * been reached during playback.
     * @param listener the callback that will be run.
     * @since 7.0
     */
    @XWalkAPI
    public void setOnCompletionListener(OnCompletionListener listener) {
        unsupported();
    }

    /**
     * Register a callback to be invoked when an error has happened during
     * an asynchronous operation.
     * @param listener the callback that will be run.
     * @since 7.0
     */
    @XWalkAPI
    public void setOnErrorListener(OnErrorListener listener) {
        unsupported();
    }

    /**
     * Register a callback to be invoked when the media source is ready for playback.
     * @param listener the callback that will be run.
     * @since 7.0
     */
    @XWalkAPI
    public void setOnPreparedListener(OnPreparedListener listener) {
        unsupported();
    }

    /**
     * Register a callback to be invoked when a seek operation has been completed.
     * @param listener the callback that will be run.
     * @since 7.0
     */
    @XWalkAPI
    public void setOnSeekCompleteListener(OnSeekCompleteListener listener) {
        unsupported();
    }

    /**
     * Register a callback to be invoked when the video size is known or updated.
     * @param listener the callback that will be run.
     * @since 7.0
     */
    @XWalkAPI
    public void setOnVideoSizeChangedListener(OnVideoSizeChangedListener listener) {
        unsupported();
    }

    /**
     * Adds an external timed text source file (FileDescriptor).
     * @param fd the FileDescriptor for the file you want to play.
     * @param mimeType The mime type of the file. Must be one of the mime types listed above.
     * @since 7.0
     */
    @XWalkAPI
    public void addTimedTextSource(FileDescriptor fd, String mimeType) {
        unsupported();
    }

    /**
     * Adds an external timed text source file.
     * @param fd The file path of external timed text source file.
     * @param mimeType The mime type of the file. Must be one of the mime types listed above.
     * @since 7.0
     */
    @XWalkAPI
    public void addTimedTextSource(String fd, String mimeType) {
        unsupported();
    }

    /**
     * Adds an external timed text file (FileDescriptor).
     * @param fd the FileDescriptor for the file you want to play.
     * @param offset the offset into the file where the data to be played starts, in bytes.
     * @param length the length in bytes of the data to be played.
     * @param mime The mime type of the file. Must be one of the mime types listed above.
     * @since 7.0
     */
    @XWalkAPI
    public void addTimedTextSource(FileDescriptor fd, long offset, long length, String mime) {
        unsupported();
    }

    /**
     * Adds an external timed text source file (Uri).
     * @param context the Context to use when resolving the Uri
     * @param uri the Content URI of the data you want to play.
     * @param mime The mime type of the file. Must be one of the mime types listed above.
     * @since 7.0
     */
    @XWalkAPI
    public void addTimedTextSource(Context context, Uri uri, String mime) {
        unsupported();
    }

    /**
     * Attaches an auxiliary effect to the player.
     * @param effectId system wide unique id of the effect to attach.
     * @since 7.0
     */
    @XWalkAPI
    public void attachAuxEffect(String fd, String mimeType) {
        unsupported();
    }

    /**
     * Currently, the track must be a timed text track and no audio or video tracks can be deselected.
     * @param index the index of the track to be deselected.
     * @since 7.0
     */
    @XWalkAPI
    public void deselectTrack(int index) {
        unsupported();
    }

    /**
     * Returns the audio session ID.
     * @return the audio session ID.
     * @since 7.0
     */
    @XWalkAPI
    public int getAudioSessionId() {
        unsupported();
        return 0;
    }

    /**
     * Gets the playback params, containing the current playback rate.
     * @return the playback params.
     * @since 7.0
     */
    // @XWalkAPI
    // public PlaybackParams getPlaybackParams() {
    //     unsupported();
    //     return null;
    // }

    /**
     * Returns the index of the audio, video, or subtitle track currently selected for playback.
     * @param trackType should be one of MEDIA_TRACK_TYPE_VIDEO, MEDIA_TRACK_TYPE_AUDIO, or MEDIA_TRACK_TYPE_SUBTITLE.
     * @return index of the audio, video, or subtitle track currently selected for playback
     * @since 7.0
     */
    @XWalkAPI
    public int getSelectedTrack(int trackType) {
        unsupported();
        return 0;
    }

    /**
     * Gets the A/V sync mode.
     * @return the A/V sync params
     * @since 7.0
     */
    // @XWalkAPI
    // public SyncParams getSyncParams() {
    //     unsupported();
    //     return null;
    // }

    /**
     * Get current playback position as a MediaTimestamp.
     * @return a MediaTimestamp object if a timestamp is available
     * @since 7.0
     */
    // @XWalkAPI
    // public MediaTimestamp getTimestamp() {
    //     unsupported();
    //     return null;
    // }

    /**
     * Checks whether the MediaPlayer is looping or non-looping.
     * @return true if the MediaPlayer is currently looping, false otherwise
     * @since 7.0
     */
    @XWalkAPI
    public boolean isLooping() {
        unsupported();
        return false;
    }

    /**
     * Prepares the player for playback.
     * @since 7.0
     */
    @XWalkAPI
    public void prepare() {
        unsupported();
    }

    /**
     * Resets the MediaPlayer to its uninitialized state.
     * @since 7.0
     */
    @XWalkAPI
    public void reset() {
        unsupported();
    }

    /**
     * If a MediaPlayer is in invalid state, it throws an IllegalStateException exception.
     * @param the index of the track to be selected.
     * @since 7.0
     */
    @XWalkAPI
    public void selectTrack(int index) {
        unsupported();
    }

    /**
     * Sets the audio attributes for this MediaPlayer.
     * @param attribute a non-null set of audio attributes.
     * @since 7.0
     */
    // @XWalkAPI
    // public void setAudioAttributes(AudioAttributes attributes) {
    //     unsupported();
    // }

    /**
     * Sets the audio session ID.
     * @param the audio session ID.
     * @since 7.0
     */
    @XWalkAPI
    public void setAudioSessionId(int sessionId) {
        unsupported();
    }

    /**
     * Sets the audio stream type for this MediaPlayer.
     * @param streamType the audio stream type.
     * @since 7.0
     */
    @XWalkAPI
    public void setAudioStreamType(int streamtype) {
        unsupported();
    }

    /**
     * Sets the send level of the player to the attached auxiliary effect.
     * @param level send level scalar.
     * @since 7.0
     */
    @XWalkAPI
    public void setAuxEffectSendLevel(float level) {
        unsupported();
    }

    /**
     * Sets the SurfaceHolder to use for displaying the video portion of the media.
     * @param sh the SurfaceHolder to use for video display.
     * @since 7.0
     */
    @XWalkAPI
    public void setDisplay(SurfaceHolder sh) {
        unsupported();
    }

    /**
     * Sets the player to be looping or non-looping.
     * @param looping whether to loop or not.
     * @since 7.0
     */
    @XWalkAPI
    public void setLooping(boolean looping) {
        unsupported();
    }

    /**
     * Set the MediaPlayer to start when this MediaPlayer finishes playback.
     * @param next the player to start after this one completes playback.
     * @since 7.0
     */
    @XWalkAPI
    public void setNextMediaPlayer(MediaPlayer next) {
        unsupported();
    }

    /**
     * Register a callback to be invoked when an info/warning is available.
     * @param listener the callback that will be run.
     * @since 7.0
     */
    @XWalkAPI
    public void setOnInfoListener(OnInfoListener listener) {
        unsupported();
    }

    /**
     * Register a callback to be invoked when a selected track has timed metadata available.
     * @param listener the callback that will be run.
     * @since 7.0
     */
    @XWalkAPI
    public void setOnTimedMetaDataAvailableListener(MediaPlayer.OnTimedMetaDataAvailableListener listener) {
        unsupported();
    }

    /**
     * Register a callback to be invoked when a timed text is available for display.
     * @param listener the callback that will be run.
     * @since 7.0
     */
    @XWalkAPI
    public void setOnTimedTextListener(OnTimedTextListener listener) {
        unsupported();
    }

    /**
     * Sets playback rate using PlaybackParams.
     * @param params the playback params.
     * @since 7.0
     */
    // @XWalkAPI
    // public void setPlaybackParams(PlaybackParams params) {
    //     unsupported();
    // }

    /**
     * Control whether we should use the attached SurfaceHolder to keep the screen
     * on while video playback is occurring.
     * @param screenOn Supply true to keep the screen on, false to allow it to turn off.
     * @since 7.0
     */
    @XWalkAPI
    public void setScreenOnWhilePlaying(boolean screenOn) {
        unsupported();
    }

    /**
     * Sets A/V sync mode.
     * @param params the A/V sync params to apply.
     * @since 7.0
     */
    // @XWalkAPI
    // public void setSyncParams(SyncParams params) {
    //     unsupported();
    // }

    /**
     * Sets video scaling mode.
     * @param mode target video scaling mode.
     * @since 7.0
     */
    @XWalkAPI
    public void setVideoScalingMode(int mode) {
        unsupported();
    }

    /**
     * Set the low-level power management behavior for this MediaPlayer.
     * @param context the Context to use.
     * @param mode the power/wake mode to set.
     * @since 7.0
     */
    @XWalkAPI
    public void setWakeMode(Context context, int mode) {
        unsupported();
    }

    /**
     * Stops playback after playback has been stopped or paused.
     * @since 7.0
     */
    @XWalkAPI
    public void stop() {
        unsupported();
    }
}
