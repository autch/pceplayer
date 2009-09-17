package net.autch.android.pceplayer;

import java.io.IOException;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class PMDPlayerThread extends Thread implements AudioTrack.OnPlaybackPositionUpdateListener {
	private static final int BYTES_PER_BLOCK = 128 * 2;
	private static final int BLOCKS_AT_ONCE = 256;
	private static final int WAIT_PER_BLOCK = 250;
	
	private final AudioTrack track;
	private boolean terminate;
	private final byte[] buffer;

	static {
		System.loadLibrary("muslib");
	}

	private static native void muslib_init();
	private static native int muslib_load_from_file(String filename);
	private static native void muslib_start();
	private static native int muslib_render(byte[] buffer, int size);
	private static native void muslib_close();

	public PMDPlayerThread() {
		track = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_CONFIGURATION_MONO,
				AudioFormat.ENCODING_PCM_16BIT,	BYTES_PER_BLOCK * BLOCKS_AT_ONCE, AudioTrack.MODE_STREAM);
		terminate = false;
		buffer = new byte[BYTES_PER_BLOCK * BLOCKS_AT_ONCE];
		muslib_init();
	}

	@Override
	protected void finalize() throws Throwable {
		muslib_close();

		super.finalize();
	}

	public void open(String filename) throws IOException {
		if(muslib_load_from_file(filename) != 1) {
			throw new IOException("Cannot open file " + filename);
		}
		muslib_start();
	}

	@Override
	public void run() {
		track.play();
		while(!terminate) {
			try {
				int ret = muslib_render(buffer, buffer.length);
				if(ret == 0) break;
				track.write(buffer, 0, buffer.length);
				Thread.sleep(WAIT_PER_BLOCK);
			} catch (InterruptedException e) {
				// thru
			}
		}
		track.stop();
		track.release();
	}

	public void terminate() {
		terminate = true;
	}

	public void onMarkerReached(AudioTrack track) {
	}

	public void onPeriodicNotification(AudioTrack track) {
		muslib_render(buffer, buffer.length);
		track.write(buffer, 0, buffer.length);
	}
}
