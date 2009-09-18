package net.autch.android.pceplayer;

public final class MusLibInterface {
	static {
		System.loadLibrary("muslib");
	}

	public static native void muslib_init();
	public static native int muslib_load_from_file(String filename);
	public static native void muslib_start();
	public static native int muslib_render(byte[] buffer, int size);
	public static native void muslib_close();
}
