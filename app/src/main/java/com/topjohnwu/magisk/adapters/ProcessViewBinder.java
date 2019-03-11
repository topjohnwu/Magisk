package com.topjohnwu.magisk.adapters;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.util.Comparator;

import butterknife.BindView;
import java9.util.Comparators;
import me.drakeet.multitype.ItemViewBinder;

public class ProcessViewBinder extends ItemViewBinder<ProcessViewBinder.Process, ProcessViewBinder.ViewHolder> {

    @Override
    protected @NonNull
    ViewHolder onCreateViewHolder(@NonNull LayoutInflater inflater, @NonNull ViewGroup parent) {
        return new ViewHolder(inflater.inflate(R.layout.list_item_hide_process, parent, false));
    }

    @Override
    protected void onBindViewHolder(@NonNull ViewHolder holder, @NonNull Process process) {
        holder.process.setText(process.name);
        holder.checkbox.setOnCheckedChangeListener(null);
        holder.checkbox.setChecked(process.hidden);
        holder.checkbox.setOnCheckedChangeListener((v, isChecked) -> {
            if (isChecked) {
                Shell.su("magiskhide --add " + process.fullname).submit();
                process.hidden = true;
            } else {
                Shell.su("magiskhide --rm " + process.fullname).submit();
                process.hidden = false;
            }

        });

    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.process) TextView process;
        @BindView(R.id.checkbox) CheckBox checkbox;

        ViewHolder(View itemView) {
            super(itemView);
            new ProcessViewBinder$ViewHolder_ViewBinding(this, itemView);
        }
    }

    public static class Process implements Comparable<Process> {
        String name;
        boolean hidden;
        String fullname;
        String packageName;

        Process(String name, boolean hidden, String packageName) {
            this.name = name;
            this.hidden = hidden;
            this.packageName=packageName;
            this.fullname = Utils.fmt("%s %s", packageName, name);
        }

        @Override
        public int compareTo(Process o) {
            Comparator<Process> c;
            c = Comparators.comparing((Process t) -> !t.name.startsWith(t.packageName));
            c = Comparators.thenComparing(c, t -> t.name, String::compareToIgnoreCase);
            return c.compare(this, o);
        }
    }
}
