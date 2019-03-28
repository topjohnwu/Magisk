package com.topjohnwu.magisk.adapters;

import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

public abstract class SectionedAdapter<S extends RecyclerView.ViewHolder, C extends RecyclerView.ViewHolder>
        extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private static final int SECTION_TYPE = Integer.MIN_VALUE;

    @NonNull
    @Override
    final public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        if (viewType == SECTION_TYPE)
            return onCreateSectionViewHolder(parent);
        return onCreateItemViewHolder(parent, viewType);
    }

    @Override
    @SuppressWarnings("unchecked")
    final public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        PositionInfo info = getPositionInfo(position);
        if (info.position == -1)
            onBindSectionViewHolder((S) holder, info.section);
        else
            onBindItemViewHolder((C) holder, info.section, info.position);
    }

    @Override
    final public int getItemCount() {
        int size, sec;
        size = sec = getSectionCount();
        for (int i = 0; i < sec; ++i){
            size += getItemCount(i);
        }
        return size;
    }

    @Override
    final public int getItemViewType(int position) {
        PositionInfo info = getPositionInfo(position);
        if (info.position == -1)
            return SECTION_TYPE;
        else
            return  getItemViewType(info.section, info.position);
    }

    public int getItemViewType(int section, int position) {
        return 0;
    }

    protected int getSectionPosition(int section) {
        return getItemPosition(section, -1);
    }

    protected int getItemPosition(int section, int position) {
        int realPosition = 0;
        // Previous sections
        for (int i = 0; i < section; ++i) {
            realPosition += getItemCount(i) + 1;
        }
        // Current section
        realPosition += position + 1;
        return realPosition;
    }

    private PositionInfo getPositionInfo(int position) {
        int section = 0;
        while (true) {
            if (position == 0)
                return new PositionInfo(section, -1);
            position -= 1;
            if (position < getItemCount(section))
                return new PositionInfo(section, position);
            position -= getItemCount(section++);
        }
    }

    private static class PositionInfo {
        int section;
        int position;
        PositionInfo(int section, int position) {
            this.section = section;
            this.position = position;
        }
    }

    public abstract int getSectionCount();
    public abstract int getItemCount(int section);
    public abstract S onCreateSectionViewHolder(ViewGroup parent);
    public abstract C onCreateItemViewHolder(ViewGroup parent, int viewType);
    public abstract void onBindSectionViewHolder(S holder, int section);
    public abstract void onBindItemViewHolder(C holder, int section, int position);
}
