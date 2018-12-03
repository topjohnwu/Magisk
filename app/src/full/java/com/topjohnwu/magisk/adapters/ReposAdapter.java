package com.topjohnwu.magisk.adapters;

import android.content.Context;
import android.database.Cursor;
import android.text.TextUtils;
import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.DownloadModule;
import com.topjohnwu.magisk.asyncs.MarkDownWindow;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.components.CustomAlertDialog;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import androidx.recyclerview.widget.RecyclerView;
import butterknife.BindView;

public class ReposAdapter extends SectionedAdapter<ReposAdapter.SectionHolder, ReposAdapter.RepoHolder> {

    private static final int UPDATES = 0;
    private static final int INSTALLED = 1;
    private static final int OTHERS = 2;

    private Cursor repoCursor = null;
    private Map<String, Module> moduleMap;
    private RepoDatabaseHelper repoDB;
    private List<Pair<Integer, List<Repo>>> repoPairs;

    public ReposAdapter(RepoDatabaseHelper db, Map<String, Module> map) {
        repoDB = db;
        moduleMap = map;
        repoPairs = new ArrayList<>();
        notifyDBChanged();
    }


    @Override
    public int getSectionCount() {
        return repoPairs.size();
    }

    @Override
    public int getItemCount(int section) {
        return repoPairs.get(section).second.size();
    }

    @Override
    public SectionHolder onCreateSectionViewHolder(ViewGroup parent) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.section, parent, false);
        return new SectionHolder(v);
    }

    @Override
    public RepoHolder onCreateItemViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_repo, parent, false);
        return new RepoHolder(v);
    }

    @Override
    public void onBindSectionViewHolder(SectionHolder holder, int section) {
        switch (repoPairs.get(section).first) {
            case UPDATES:
                holder.sectionText.setText(R.string.update_available);
                break;
            case INSTALLED:
                holder.sectionText.setText(R.string.installed);
                break;
            case OTHERS:
                holder.sectionText.setText(R.string.not_installed);
                break;
        }
    }

    @Override
    public void onBindItemViewHolder(RepoHolder holder, int section, int position) {
        Repo repo = repoPairs.get(section).second.get(position);
        Context context = holder.itemView.getContext();

        String name = repo.getName();
        String version = repo.getVersion();
        String author = repo.getAuthor();
        String description = repo.getDescription();
        String noInfo = context.getString(R.string.no_info_provided);

        holder.title.setText(TextUtils.isEmpty(name) ? noInfo : name);
        holder.versionName.setText(TextUtils.isEmpty(version) ? noInfo : version);
        holder.author.setText(TextUtils.isEmpty(author) ? noInfo : context.getString(R.string.author, author));
        holder.description.setText(TextUtils.isEmpty(description) ? noInfo : description);
        holder.updateTime.setText(context.getString(R.string.updated_on, repo.getLastUpdateString()));

        holder.infoLayout.setOnClickListener(v ->
                new MarkDownWindow((BaseActivity) context, null, repo.getDetailUrl()).exec());

        holder.downloadImage.setOnClickListener(v -> {
            new CustomAlertDialog((BaseActivity) context)
                .setTitle(context.getString(R.string.repo_install_title, repo.getName()))
                .setMessage(context.getString(R.string.repo_install_msg, repo.getDownloadFilename()))
                .setCancelable(true)
                .setPositiveButton(R.string.install, (d, i) ->
                        DownloadModule.exec((BaseActivity) context, repo, true))
                .setNeutralButton(R.string.download, (d, i) ->
                        DownloadModule.exec((BaseActivity) context, repo, false))
                .setNegativeButton(R.string.no_thanks, null)
                .show();
        });
    }

    public void notifyDBChanged() {
        if (repoCursor != null)
            repoCursor.close();
        repoCursor = repoDB.getRepoCursor();
        filter("");
    }

    public void filter(String s) {
        List<Repo> updates = new ArrayList<>();
        List<Repo> installed = new ArrayList<>();
        List<Repo> others = new ArrayList<>();

        repoPairs.clear();
        while (repoCursor.moveToNext()) {
            Repo repo = new Repo(repoCursor);
            if (repo.getName().toLowerCase().contains(s.toLowerCase())
                    || repo.getAuthor().toLowerCase().contains(s.toLowerCase())
                    || repo.getDescription().toLowerCase().contains(s.toLowerCase())
                    ) {
                // Passed the repoFilter
                Module module = moduleMap.get(repo.getId());
                if (module != null) {
                    if (repo.getVersionCode() > module.getVersionCode()) {
                        // Updates
                        updates.add(repo);
                    } else {
                        installed.add(repo);
                    }
                } else {
                    others.add(repo);
                }
            }
        }
        repoCursor.moveToFirst();

        if (!updates.isEmpty())
            repoPairs.add(new Pair<>(UPDATES, updates));
        if (!installed.isEmpty())
            repoPairs.add(new Pair<>(INSTALLED, installed));
        if (!others.isEmpty())
            repoPairs.add(new Pair<>(OTHERS, others));

        notifyDataSetChanged();
    }

    static class SectionHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.section_text) TextView sectionText;

        SectionHolder(View itemView) {
            super(itemView);
            new ReposAdapter$SectionHolder_ViewBinding(this, itemView);
        }
    }

    static class RepoHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title) TextView title;
        @BindView(R.id.version_name) TextView versionName;
        @BindView(R.id.description) TextView description;
        @BindView(R.id.author) TextView author;
        @BindView(R.id.info_layout) LinearLayout infoLayout;
        @BindView(R.id.download) ImageView downloadImage;
        @BindView(R.id.update_time) TextView updateTime;

        RepoHolder(View itemView) {
            super(itemView);
            new ReposAdapter$RepoHolder_ViewBinding(this, itemView);
        }

    }
}
