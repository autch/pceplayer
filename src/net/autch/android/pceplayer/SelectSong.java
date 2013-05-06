package net.autch.android.pceplayer;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.BaseColumns;
import android.provider.MediaStore;
import android.provider.MediaStore.MediaColumns;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;

public class SelectSong extends ListActivity {
	private static final String TAG = "SelectSong";

	private static final int MID_STOP = 0x1001;

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
		final SelectSong me = this;
		super.onCreate(savedInstanceState);

		files.clear();

		dialog = ProgressDialog.show(this, null, "曲を探しています...", true, false);
		final ContentResolver cr = this.getContentResolver();
		new Thread(new Runnable() {
			public void run() {
				Uri uri = MediaStore.Files.getContentUri("external");
				String[] projection = { BaseColumns._ID, MediaColumns.DATA };
				String where = MediaStore.Files.FileColumns.MEDIA_TYPE + "="
						+ MediaStore.Files.FileColumns.MEDIA_TYPE_NONE +
						" AND LOWER(" + MediaStore.Files.FileColumns.DATA + ") LIKE '%.pmd'";
				Cursor mediaFiles = cr.query(uri, projection, where, null, MediaColumns.DATA);
				try {
					int dataCol = mediaFiles
							.getColumnIndexOrThrow(MediaColumns.DATA);
					while (mediaFiles.moveToNext()) {
						String filename = mediaFiles.getString(dataCol);
						File f = new File(filename);
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
						} catch (/* IO */Exception e) {
							// TODO 自動生成された catch ブロック
							// e.printStackTrace();
							// return false;
						}
					}
					handler.post(new Runnable() {
						public void run() {
							SimpleAdapter adapter = new SimpleAdapter(
									SelectSong.this, files,
									android.R.layout.simple_list_item_2,
									new String[] { "title", "title2" },
									new int[] { android.R.id.text1,
											android.R.id.text2 });
							setListAdapter(adapter);
							getListView().setFastScrollEnabled(true);
							getListView().getParent().requestLayout();
							dialog.dismiss();
						}
					});
				} finally {
					mediaFiles.close();
				}
			}
		}).start();
/*
		new Thread(new Runnable() {
			public void run() {
				DirDiver diver = new DirDiver(Environment.getExternalStorageDirectory().getAbsolutePath(), ".pmd");
				diver.setCallback(enumFiles);
				diver.start();

				handler.post(new Runnable() {
					public void run() {
						SimpleAdapter adapter = new SimpleAdapter(SelectSong.this, files, android.R.layout.simple_list_item_2,
								new String[] { "title", "title2" }, new int[] { android.R.id.text1, android.R.id.text2 });
						setListAdapter(adapter);
						getListView().setFastScrollEnabled(true);
						getListView().getParent().requestLayout();
						dialog.dismiss();
					}
				});
			}
		}).start();
*/
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		HashMap<String, String> item = files.get(position);
		Intent it = new Intent(PMDPlayerService.ACTION_PLAY);
		it.putExtra("filename", item.get("filename"));
		it.putExtra("title", item.get("title"));
		it.putExtra("title2", item.get("title2"));
		startService(it);
		Toast t = Toast.makeText(this, "演奏を開始します", Toast.LENGTH_SHORT);
		t.show();
	}

	@Override
	protected void onPause() {
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		super.onCreateOptionsMenu(menu);
		menu.add(Menu.NONE, MID_STOP, Menu.NONE, "停止");
		return true;
	}

	@Override
	public boolean onMenuItemSelected(int featureId, MenuItem item) {
		switch (item.getItemId()) {
		case MID_STOP:
			startService(new Intent(PMDPlayerService.ACTION_STOP));
			Toast t = Toast.makeText(this, "停止しました", Toast.LENGTH_SHORT);
			t.show();
		}
		return super.onMenuItemSelected(featureId, item);
	}
}
