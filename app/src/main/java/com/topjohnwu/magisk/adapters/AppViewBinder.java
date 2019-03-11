package com.topjohnwu.magisk.adapters;

import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;

import com.buildware.widget.indeterm.IndeterminateCheckBox;
import com.topjohnwu.magisk.R;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

import butterknife.BindView;
import java9.util.Comparators;
import me.drakeet.multitype.ItemViewBinder;

public class AppViewBinder extends ItemViewBinder<AppViewBinder.App, AppViewBinder.ViewHolder> {

    private final List<Object> items;

    AppViewBinder(List<Object> items) {
        this.items = items;
    }


    @Override
    protected @NonNull
    ViewHolder onCreateViewHolder(@NonNull LayoutInflater inflater, @NonNull ViewGroup parent) {
        return new ViewHolder(inflater.inflate(R.layout.list_item_hide_app, parent, false));
    }

    @Override
    protected void onBindViewHolder(@NonNull ViewHolder holder, @NonNull App app) {
        IndeterminateCheckBox.OnStateChangedListener listener =
                (IndeterminateCheckBox indeterminateCheckBox, @Nullable Boolean stat) -> {
                    if (stat != null && stat) {
                        for (ProcessViewBinder.Process process : app.processes) {
                            Shell.su("magiskhide --add " + process.fullname).submit();
                            process.hidden = true;
                        }
                    } else if (stat != null) {
                        for (ProcessViewBinder.Process process : app.processes) {
                            Shell.su("magiskhide --rm " + process.fullname).submit();
                            process.hidden = false;
                        }
                    }
                    app.getStatBool(true);
                };
        holder.app_name.setText(app.name);
        holder.app_icon.setImageDrawable(app.icon);
        holder.package_name.setText(app.packageName);
        holder.checkBox.setOnStateChangedListener(null);
        holder.checkBox.setState(app.getStatBool(false));
        holder.checkBox.setOnStateChangedListener(listener);
        if (app.expand) {
            holder.checkBox.setVisibility(View.GONE);
            setBottomMargin(holder.itemView, 0);
        } else {
            holder.checkBox.setVisibility(View.VISIBLE);
            setBottomMargin(holder.itemView, 2);
        }
        holder.itemView.setOnClickListener((v) -> {
            int index = getPosition(holder);
            if (app.expand) {
                app.expand = false;
                items.removeAll(app.processes);
                getAdapter().notifyItemRangeRemoved(index + 1, app.processes.size());
                setBottomMargin(holder.itemView, 2);
                holder.checkBox.setOnStateChangedListener(null);
                holder.checkBox.setVisibility(View.VISIBLE);
                holder.checkBox.setState(app.getStatBool(true));
                holder.checkBox.setOnStateChangedListener(listener);
            } else {
                holder.checkBox.setVisibility(View.GONE);
                setBottomMargin(holder.itemView, 0);
                app.expand = true;
                items.addAll(index + 1, app.processes);
                getAdapter().notifyItemRangeInserted(index + 1, app.processes.size());
            }
        });
    }

    private void setBottomMargin(View view, int dp) {
        ViewGroup.LayoutParams params = view.getLayoutParams();
        ViewGroup.MarginLayoutParams marginParams;
        if (params instanceof ViewGroup.MarginLayoutParams) {
            marginParams = (ViewGroup.MarginLayoutParams) params;
        } else {
            marginParams = new ViewGroup.MarginLayoutParams(params);
        }
        int px = (int) (0.5f + dp * Resources.getSystem().getDisplayMetrics().density);
        marginParams.bottomMargin = px;
        view.setLayoutParams(marginParams);
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_icon) ImageView app_icon;
        @BindView(R.id.app_name) TextView app_name;
        @BindView(R.id.package_name) TextView package_name;
        @BindView(R.id.checkbox) IndeterminateCheckBox checkBox;

        ViewHolder(View itemView) {
            super(itemView);
            new AppViewBinder$ViewHolder_ViewBinding(this, itemView);
        }
    }

    public static class App implements Comparable<App> {
        Drawable icon;
        String name;
        String packageName;
        int flags;
        List<ProcessViewBinder.Process> processes;
        boolean expand = false;
        int stat = -1;

        App(Drawable icon, String name, String packageName, int flags) {
            this.icon = icon;
            this.name = name;
            this.packageName = packageName;
            this.flags=flags;
            this.processes = new ArrayList<>();
        }

        int getStat(boolean update) {
            if (stat > 1 && !update) return stat;
            int n = 0;
            for (ProcessViewBinder.Process process : processes) {
                if (process.hidden) n++;
            }
            if (n == processes.size()) stat = 2;
            else if (n > 0) stat = 1;
            else stat = 0;
            return stat;
        }

        Boolean getStatBool(boolean update) {
            int stat = getStat(update);
            switch (stat) {
                case 2:
                    return true;
                case 1:
                    return null;
                case 0:
                default:
                    return false;
            }
        }

        @Override
        public int compareTo(App o) {
            Comparator<App> c;
            c = Comparators.comparing((App t) -> t.stat);
            c = Comparators.reversed(c);
            c = Comparators.thenComparing(c, t -> t.name, String::compareToIgnoreCase);
            return c.compare(this, o);
        }
    }
}
