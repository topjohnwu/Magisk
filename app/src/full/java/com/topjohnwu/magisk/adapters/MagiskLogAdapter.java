package com.topjohnwu.magisk.adapters;

import android.app.Activity;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.topjohnwu.magisk.R;

import java.util.List;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import butterknife.BindView;

public class MagiskLogAdapter extends RecyclerView.Adapter<MagiskLogAdapter.ViewHolder> {

    private List<String> mList;
    private int txtWidth = -1;

    public MagiskLogAdapter(List<String> list) {
        mList = list;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.list_item_magisk_log, parent, false);
        ViewHolder vh = new ViewHolder(v);
        if (txtWidth < 0) {
            int max = 0;
            String maxStr = "";
            for (String s : mList) {
                int len = s.length();
                if (len > max) {
                    max = len;
                    maxStr = s;
                }
            }
            vh.txt.setText(maxStr);
            vh.txt.measure(0, 0);
            txtWidth = vh.txt.getMeasuredWidth();
            DisplayMetrics displayMetrics = new DisplayMetrics();
            ((Activity) parent.getContext()).getWindowManager()
                    .getDefaultDisplay().getMetrics(displayMetrics);
            if (txtWidth < displayMetrics.widthPixels)
                txtWidth = displayMetrics.widthPixels;
        }
        vh.txt.getLayoutParams().width = txtWidth;
        return vh;
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        holder.txt.setText(mList.get(position));
    }

    @Override
    public int getItemCount() {
        return mList.size();
    }

    public class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.txt) TextView txt;

        public ViewHolder(@NonNull View v) {
            super(v);
            new MagiskLogAdapter$ViewHolder_ViewBinding(this, itemView);
        }
    }
}
