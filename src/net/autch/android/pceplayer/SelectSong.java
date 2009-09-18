package net.autch.android.pceplayer;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.widget.ListView;
import android.widget.SimpleAdapter;

public class SelectSong extends ListActivity {
	private final ArrayList<HashMap<String, String>> files = new ArrayList<HashMap<String, String>>();
	private final Handler handler = new Handler();
	private ProgressDialog dialog; 

	private final DirDiver.Callback enumFiles = new DirDiver.Callback() {
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
				return false;
			}
			return true;
		}
	}; 

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		files.clear();

		dialog = ProgressDialog.show(this, null, "曲を探しています...", true, false);

		new Thread(new Runnable() {
			public void run() {
				DirDiver diver = new DirDiver("/sdcard", ".pmd");
				diver.setCallback(enumFiles);
				diver.start();

				handler.post(new Runnable() {
					public void run() {
						SimpleAdapter adapter = new SimpleAdapter(SelectSong.this, files, android.R.layout.simple_list_item_2,
								new String[] { "title", "title2" }, new int[] { android.R.id.text1, android.R.id.text2 });
						setListAdapter(adapter);
						dialog.dismiss();
					}
				});
			}
		}).start();
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
