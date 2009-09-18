package net.autch.android.pceplayer;

import java.io.IOException;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class PMDPlayerThread extends Thread {
	private static final int BYTES_PER_BLOCK = 128 * 2;
	private static final int BLOCKS_AT_ONCE = 256;
	private static final int WAIT_PER_BLOCK = 250;

	private final AudioTrack track;
	private boolean terminate;
	private final byte[] buffer;

	public PMDPlayerThread() {
		track = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_CONFIGURATION_MONO,
				AudioFormat.ENCODING_PCM_16BIT,	BYTES_PER_BLOCK * BLOCKS_AT_ONCE, AudioTrack.MODE_STREAM);
		terminate = false;
		buffer = new byte[BYTES_PER_BLOCK * BLOCKS_AT_ONCE];
		MusLibInterface.muslib_init();
	}

	@Override
	protected void finalize() throws Throwable {
		MusLibInterface.muslib_close();

		super.finalize();
	}

	public void open(String filename) throws IOException {
		if(MusLibInterface.muslib_load_from_file(filename) != 1) {
			throw new IOException("Cannot open file " + filename);
		}
		MusLibInterface.muslib_start();
	}

	@Override
	public void run() {
		track.play();
		while(!terminate) {
			try {
				int ret = MusLibInterface.muslib_render(buffer, buffer.length);
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
		MusLibInterface.muslib_render(buffer, buffer.length);
		track.write(buffer, 0, buffer.length);
	}
}
