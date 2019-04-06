package com.topjohnwu.magisk.adapters;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.Build;
import android.text.TextUtils;
import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SearchView;
import android.widget.TextView;

import androidx.recyclerview.widget.RecyclerView;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.components.DownloadModuleService;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.dialogs.CustomAlertDialog;
import com.topjohnwu.magisk.uicomponents.MarkDownWindow;
import com.topjohnwu.magisk.utils.Event;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import java9.util.stream.StreamSupport;

public class ReposAdapter
        extends SectionedAdapter<ReposAdapter.SectionHolder, ReposAdapter.RepoHolder>
        implements Event.AutoListener, SearchView.OnQueryTextListener {

    private static final int UPDATES = 0;
    private static final int INSTALLED = 1;
    private static final int OTHERS = 2;

    private Map<String, Module> moduleMap;
    private RepoDatabaseHelper repoDB;
    private List<Pair<Integer, List<Repo>>> repoPairs;
    private List<Repo> fullList;
    private SearchView mSearch;

    public ReposAdapter() {
        repoDB = App.self.repoDB;
        moduleMap = Collections.emptyMap();
        fullList = Collections.emptyList();
        repoPairs = new ArrayList<>();
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
                MarkDownWindow.show((BaseActivity) context, null, repo.getDetailUrl()));

        holder.downloadImage.setOnClickListener(v -> {
            new CustomAlertDialog((BaseActivity) context)
                .setTitle(context.getString(R.string.repo_install_title, repo.getName()))
                .setMessage(context.getString(R.string.repo_install_msg, repo.getDownloadFilename()))
                .setCancelable(true)
                .setPositiveButton(R.string.install, (d, i) ->
                        startDownload((BaseActivity) context, repo, true))
                .setNeutralButton(R.string.download, (d, i) ->
                        startDownload((BaseActivity) context, repo, false))
                .setNegativeButton(R.string.no_thanks, null)
                .show();
        });
    }

    private void startDownload(BaseActivity activity, Repo repo, Boolean install) {
        activity.runWithExternalRW(() -> {
            Intent intent = new Intent(activity, ClassMap.get(DownloadModuleService.class))
                    .putExtra("repo", repo).putExtra("install", install);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                activity.startForegroundService(intent);
            } else {
                activity.startService(intent);
            }
        });
    }

    private void updateLists() {
        if (mSearch != null)
            onQueryTextChange(mSearch.getQuery().toString());
        else
            onQueryTextChange("");
    }

    private static boolean noCaseContain(String a, String b) {
        return a.toLowerCase().contains(b.toLowerCase());
    }

    public void setSearchView(SearchView view) {
        mSearch = view;
        mSearch.setOnQueryTextListener(this);
    }

    public void notifyDBChanged(boolean refresh) {
        try (Cursor c = repoDB.getRepoCursor()) {
            fullList = new ArrayList<>(c.getCount());
            while (c.moveToNext())
                fullList.add(new Repo(c));
        }
        if (refresh)
            updateLists();
    }

    @Override
    public void onEvent(int event) {
        moduleMap = Event.getResult(event);
        updateLists();
    }

    @Override
    public int[] getListeningEvents() {
        return new int[] {Event.MODULE_LOAD_DONE};
    }

    @Override
    public boolean onQueryTextSubmit(String query) {
        return false;
    }

    @Override
    public boolean onQueryTextChange(String s) {
        List<Repo> updates = new ArrayList<>();
        List<Repo> installed = new ArrayList<>();
        List<Repo> others = new ArrayList<>();

        StreamSupport.stream(fullList)
                .filter(repo -> noCaseContain(repo.getName(), s)
                        || noCaseContain(repo.getAuthor(), s)
                        || noCaseContain(repo.getDescription(), s))
                .forEach(repo -> {
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
                });

        repoPairs.clear();
        if (!updates.isEmpty())
            repoPairs.add(new Pair<>(UPDATES, updates));
        if (!installed.isEmpty())
            repoPairs.add(new Pair<>(INSTALLED, installed));
        if (!others.isEmpty())
            repoPairs.add(new Pair<>(OTHERS, others));

        notifyDataSetChanged();
        return false;
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
        @BindView(R.id.info_layout) View infoLayout;
        @BindView(R.id.download) ImageView downloadImage;
        @BindView(R.id.update_time) TextView updateTime;

        RepoHolder(View itemView) {
            super(itemView);
            new ReposAdapter$RepoHolder_ViewBinding(this, itemView);
        }

    }
}
