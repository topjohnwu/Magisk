package com.topjohnwu.magisk.rv;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.model.Module;

import java.util.List;

public class ModulesAdapter extends ArrayAdapter<Module> {

    public ModulesAdapter(Context context, int resource, List<Module> modules) {
        super(context, resource, modules);
    }

    @SuppressLint("SetTextI18n")
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder vh;

        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater) getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.row, null);

            vh = new ViewHolder();
            vh.name = (TextView) convertView.findViewById(R.id.name);
            vh.version = (TextView) convertView.findViewById(R.id.version);
            vh.versionCode = (TextView) convertView.findViewById(R.id.versionCode);
            vh.description = (TextView) convertView.findViewById(R.id.description);
            vh.cache = (TextView) convertView.findViewById(R.id.cache);

            convertView.setTag(vh);
        } else {
            vh = (ViewHolder) convertView.getTag();
        }

        Module module = getItem(position);
        vh.name.setText("name= " + module.getName());
        vh.version.setText("version= " + module.getVersion());
        vh.versionCode.setText("versioncode= " + module.getVersionCode());
        vh.description.setText("description= " + module.getDescription());
        vh.cache.setText("is from cache= " + module.isCache());

        return convertView;
    }

    static class ViewHolder {
        public TextView name;
        public TextView version;
        public TextView versionCode;
        public TextView description;
        public TextView cache;
    }
}