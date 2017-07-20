package com.topjohnwu.magisk.adapters;

import android.app.Activity;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.ProcessRepoZip;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.components.MarkDownWindow;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private static final int SECTION_TYPE = 0;
    private static final int REPO_TYPE = 1;

    private List<Repo> mUpdateRepos, mInstalledRepos, mOthersRepos;
    private int[] sectionList;
    private int size;
    private Cursor repoCursor = null;
    private Map<String, Module> moduleMap;
    private RepoDatabaseHelper repoDB;

    public ReposAdapter(RepoDatabaseHelper db, Map<String, Module> map) {
        repoDB = db;
        moduleMap = map;
        mUpdateRepos = new ArrayList<>();
        mInstalledRepos = new ArrayList<>();
        mOthersRepos = new ArrayList<>();
        sectionList = new int[3];
        size = 0;
    }

    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        Context context = parent.getContext();
        View v;
        RecyclerView.ViewHolder holder = null;
        switch (viewType) {
            case SECTION_TYPE:
                v = LayoutInflater.from(context).inflate(R.layout.section, parent, false);
                holder = new SectionHolder(v);
                break;
            case REPO_TYPE:
                v = LayoutInflater.from(context).inflate(R.layout.list_item_repo, parent, false);
                holder = new RepoHolder(v);
                break;
        }
        return holder;
    }

    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
        Context context = holder.itemView.getContext();
        switch (getItemViewType(position)) {
            case SECTION_TYPE:
                SectionHolder section = (SectionHolder) holder;
                if (position == sectionList[0]) {
                    section.sectionText.setText(context.getString(R.string.update_available));
                } else if (position == sectionList[1]) {
                    section.sectionText.setText(context.getString(R.string.installed));
                } else {
                    section.sectionText.setText(context.getString(R.string.not_installed));
                }
                break;
            case REPO_TYPE:
                RepoHolder repoHolder = (RepoHolder) holder;
                Repo repo = getRepo(position);
                repoHolder.title.setText(repo.getName());
                repoHolder.versionName.setText(repo.getVersion());
                String author = repo.getAuthor();
                repoHolder.author.setText(TextUtils.isEmpty(author) ? null : context.getString(R.string.author, author));
                repoHolder.description.setText(repo.getDescription());

                repoHolder.infoLayout.setOnClickListener(v -> new MarkDownWindow(null, repo.getDetailUrl(), context));

                repoHolder.downloadImage.setOnClickListener(v -> {
                    String filename = repo.getName() + "-" + repo.getVersion() + ".zip";
                    new AlertDialogBuilder(context)
                            .setTitle(context.getString(R.string.repo_install_title, repo.getName()))
                            .setMessage(context.getString(R.string.repo_install_msg, filename))
                            .setCancelable(true)
                            .setPositiveButton(R.string.install, (d, i) -> Utils.dlAndReceive(
                                    context,
                                    new DownloadReceiver() {
                                        @Override
                                        public void onDownloadDone(Uri uri) {
                                            new ProcessRepoZip((Activity) context, uri, true).exec();
                                        }
                                    },
                                    repo.getZipUrl(),
                                    Utils.getLegalFilename(filename)))
                            .setNeutralButton(R.string.download, (d, i) -> Utils.dlAndReceive(
                                    context,
                                    new DownloadReceiver() {
                                        @Override
                                        public void onDownloadDone(Uri uri) {
                                            new ProcessRepoZip((Activity) context, uri, false).exec();
                                        }
                                    },
                                    repo.getZipUrl(),
                                    Utils.getLegalFilename(filename)))
                            .setNegativeButton(R.string.no_thanks, null)
                            .show();
                });
                break;
        }
    }

    @Override
    public int getItemViewType(int position) {
        for (int i : sectionList) {
            if (position == i)
                return SECTION_TYPE;
        }
        return REPO_TYPE;
    }

    @Override
    public int getItemCount() {
        return size;
    }

    public void notifyDBChanged() {
        if (repoCursor != null)
            repoCursor.close();
        repoCursor = repoDB.getRepoCursor();
        filter("");
    }

    public void filter(String s) {
        mUpdateRepos.clear();
        mInstalledRepos.clear();
        mOthersRepos.clear();
        sectionList[0] = sectionList[1] = sectionList[2] = 0;
        while (repoCursor.moveToNext()) {
            Repo repo = new Repo(repoCursor);
            if (repo.getName().toLowerCase().contains(s.toLowerCase())
                    || repo.getAuthor().toLowerCase().contains(s.toLowerCase())
                    || repo.getDescription().toLowerCase().contains(s.toLowerCase())
                    ) {
                // Passed the filter
                Module module = moduleMap.get(repo.getId());
                if (module != null) {
                    if (repo.getVersionCode() > module.getVersionCode()) {
                        // Updates
                        mUpdateRepos.add(repo);
                    } else {
                        mInstalledRepos.add(repo);
                    }
                } else {
                    mOthersRepos.add(repo);
                }
            }
        }
        repoCursor.moveToFirst();

        sectionList[0] = mUpdateRepos.isEmpty() ? -1 : 0;
        size = mUpdateRepos.isEmpty() ? 0 : mUpdateRepos.size() + 1;
        sectionList[1] = mInstalledRepos.isEmpty() ? -1 : size;
        size += mInstalledRepos.isEmpty() ? 0 : mInstalledRepos.size() + 1;
        sectionList[2] = mOthersRepos.isEmpty() ? -1 : size;
        size += mOthersRepos.isEmpty() ? 0 : mOthersRepos.size() + 1;

        notifyDataSetChanged();
    }

    private Repo getRepo(int position) {
        if (!mUpdateRepos.isEmpty()) position -= 1;
        if (position < mUpdateRepos.size()) return mUpdateRepos.get(position);
        position -= mUpdateRepos.size();
        if (!mInstalledRepos.isEmpty()) position -= 1;
        if (position < mInstalledRepos.size()) return mInstalledRepos.get(position);
        position -= mInstalledRepos.size();
        if (!mOthersRepos.isEmpty()) position -= 1;
        return mOthersRepos.get(position);
    }

    static class SectionHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.section_text) TextView sectionText;

        SectionHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }
    }

    static class RepoHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title) TextView title;
        @BindView(R.id.version_name) TextView versionName;
        @BindView(R.id.description) TextView description;
        @BindView(R.id.author) TextView author;
        @BindView(R.id.info_layout) LinearLayout infoLayout;
        @BindView(R.id.download) ImageView downloadImage;

        RepoHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }

    }
}
