package com.topjohnwu.magisk.adapters;

import android.app.Activity;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.IdRes;
import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.List;

public abstract class StringListAdapter<VH extends StringListAdapter.ViewHolder>
        extends RecyclerView.Adapter<VH> {

    private RecyclerView rv;
    private boolean dynamic;
    private int screenWidth;
    private int txtWidth = -1;
    private int padding;

    protected List<String> mList;

    public StringListAdapter(List<String> list) {
        this(list, false);
    }

    public StringListAdapter(List<String> list, boolean isDynamic) {
        mList = list;
        dynamic = isDynamic;
    }

    @NonNull
    @Override
    public final VH onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(itemLayoutRes(), parent, false);
        VH vh = createViewHolder(v);
        if (txtWidth < 0)
            onUpdateTextWidth(vh);
        return vh;
    }

    @Override
    public void onBindViewHolder(@NonNull VH holder, int position) {
        holder.txt.setText(mList.get(position));
        holder.txt.getLayoutParams().width = txtWidth;
        if (dynamic)
            onUpdateTextWidth(holder);
    }

    protected void onUpdateTextWidth(VH vh) {
        if (txtWidth < 0) {
            txtWidth = screenWidth - padding;
        } else {
            vh.txt.measure(0, 0);
            int width = vh.txt.getMeasuredWidth();
            if (width > txtWidth) {
                txtWidth = width;
                vh.txt.getLayoutParams().width = txtWidth;
            }
        }
        if (rv.getWidth() != txtWidth + padding)
            rv.requestLayout();
    }

    @Override
    public void onAttachedToRecyclerView(@NonNull RecyclerView rv) {
        DisplayMetrics displayMetrics = new DisplayMetrics();
        ((Activity) rv.getContext()).getWindowManager()
                .getDefaultDisplay().getMetrics(displayMetrics);
        screenWidth = displayMetrics.widthPixels;
        padding = rv.getPaddingStart() + rv.getPaddingEnd();
        this.rv = rv;
    }

    @Override
    public final int getItemCount() {
        return mList.size();
    }

    @LayoutRes
    protected abstract int itemLayoutRes();

    @NonNull
    public abstract VH createViewHolder(@NonNull View v);

    public static abstract class ViewHolder extends RecyclerView.ViewHolder {

        public TextView txt;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            txt = itemView.findViewById(textViewResId());
        }

        @IdRes
        protected abstract int textViewResId();
    }
}
