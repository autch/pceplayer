package net.autch.android.pceplayer;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ListView;
import android.widget.SimpleAdapter;

public class SelectSong extends ListActivity {
	ArrayList<HashMap<String, String>> files;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		files = new ArrayList<HashMap<String, String>>();

		DirDiver diver = new DirDiver("/sdcard", ".pmd");
		diver.setCallback(new DirDiver.Callback() {
			public boolean process(File f) {
				try {
					String[] titles = PMDFileParser.getTitleInfo(f);
					String realTitle = titles[0];
					if(realTitle == null || realTitle.length() == 0) {
						realTitle = f.getName();
					}

					HashMap<String, String> map = new HashMap<String, String>();
					map.put("filename", f.toString());
					map.put("title", realTitle);
					map.put("title2", titles[1]);
					files.add(map);
				} catch (IOException e) {
					// TODO 自動生成された catch ブロック
					e.printStackTrace();
				}
				return true;
			}
		});
		diver.start();

		SimpleAdapter adapter = new SimpleAdapter(this, files, android.R.layout.simple_list_item_2,
				new String[] { "title", "title2" }, new int[] { android.R.id.text1, android.R.id.text2 });
		setListAdapter(adapter);
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		HashMap<String, String> item = files.get(position);

		Intent it = new Intent(this, PMDPlayerService.class);
		it.putExtra("filename", item.get("filename"));
		it.putExtra("title", item.get("title"));
		it.putExtra("title2", item.get("title2"));
		startService(it);

		it = new Intent(this, PlayingView.class);
		startActivity(it);
	}
}
