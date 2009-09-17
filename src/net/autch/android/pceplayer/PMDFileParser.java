package net.autch.android.pceplayer;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.nio.charset.Charset;

public class PMDFileParser {

	public static String[] getTitleInfo(File f) throws IOException {
		String[] titles = new String[] { "", "" };
		ByteBuffer buffer = ByteBuffer.allocate(1024);
		FileInputStream is = new FileInputStream(f);
		FileChannel ch = is.getChannel();
	
		buffer.order(ByteOrder.LITTLE_ENDIAN);
		
		ch.position(0);
		ch.read(buffer);
		buffer.rewind();
		
		int parts = buffer.get();	// 0, or # of parts
		if(parts == 0) {
			parts = buffer.get();
		}
		for(; parts > 0; parts--) {
			buffer.getShort();	// addr to part data
		}
		buffer.getShort();	// addr to drum definition
		
		int adrTitle, adrTitle2;
		int lenTitle, lenTitle2;
		adrTitle = buffer.getShort();
		adrTitle2 = buffer.getShort();
		
		if(adrTitle > 0) {
			buffer.position(adrTitle);
			for(lenTitle = 0; buffer.get() != 0x00; lenTitle++)
				;
			buffer.position(adrTitle);
			byte[] byTitle = new byte[lenTitle];
			buffer.get(byTitle);
			titles[0] = bytesToString(byTitle);
		}
		if(adrTitle2 > 0) {
			buffer.position(adrTitle2);
			for(lenTitle2 = 0; buffer.get() != 0x00; lenTitle2++)
				;
			buffer.position(adrTitle2);
			byte[] byTitle2 = new byte[lenTitle2];
			buffer.get(byTitle2);
			titles[1] = bytesToString(byTitle2);
		}
		
		return titles;
	}
	
	private static String bytesToString(byte[] b) throws IOException {
		BufferedReader br = new BufferedReader(new InputStreamReader(new ByteArrayInputStream(b), Charset.forName("Shift_JIS")), 1024);
		return br.readLine();
	}
}
