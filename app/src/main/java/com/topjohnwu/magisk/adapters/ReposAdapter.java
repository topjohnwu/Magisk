package com.topjohnwu.magisk.adapters;

import android.app.Activity;
import android.content.Context;
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
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposAdapter extends RecyclerView.Adapter<ReposAdapter.ViewHolder> {

    private List<Repo> mUpdateRepos, mInstalledRepos, mOthersRepos;
    private Context mContext;

    public ReposAdapter(List<Repo> update, List<Repo> installed, List<Repo> others) {
        mUpdateRepos = update;
        mInstalledRepos = installed;
        mOthersRepos = others;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        mContext = parent.getContext();
        View v = LayoutInflater.from(mContext).inflate(R.layout.list_item_repo, parent, false);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        Repo repo = getItem(position);

        holder.title.setText(repo.getName());
        holder.versionName.setText(repo.getVersion());
        String author = repo.getAuthor();
        holder.author.setText(TextUtils.isEmpty(author) ? null : mContext.getString(R.string.author, author));
        holder.description.setText(repo.getDescription());

        holder.infoLayout.setOnClickListener(v -> new MarkDownWindow(null, repo.getDetailUrl(), mContext));

        holder.downloadImage.setOnClickListener(v -> {
            String filename = repo.getName() + "-" + repo.getVersion() + ".zip";
            new AlertDialogBuilder(mContext)
                    .setTitle(mContext.getString(R.string.repo_install_title, repo.getName()))
                    .setMessage(mContext.getString(R.string.repo_install_msg, filename))
                    .setCancelable(true)
                    .setPositiveButton(R.string.install, (d, i) -> Utils.dlAndReceive(
                            mContext,
                            new DownloadReceiver() {
                                @Override
                                public void onDownloadDone(Uri uri) {
                                    Activity activity = (Activity) mContext;
                                    new ProcessRepoZip(activity, uri, true).exec();
                                }
                            },
                            repo.getZipUrl(),
                            Utils.getLegalFilename(filename)))
                    .setNeutralButton(R.string.download, (d, i) -> Utils.dlAndReceive(
                            mContext,
                            new DownloadReceiver() {
                                @Override
                                public void onDownloadDone(Uri uri) {
                                    Activity activity = (Activity) mContext;
                                    new ProcessRepoZip(activity, uri, false).exec();
                                }
                            },
                            repo.getZipUrl(),
                            Utils.getLegalFilename(filename)))
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        });
    }

    @Override
    public int getItemCount() {
        return mUpdateRepos.size() + mInstalledRepos.size() + mOthersRepos.size();
    }

    private Repo getItem(int position) {
        if (position >= mUpdateRepos.size()) {
            position -= mUpdateRepos.size();
            if (position >= mInstalledRepos.size()) {
                position -= mInstalledRepos.size();
                return mOthersRepos.get(position);
            } else {
                return mInstalledRepos.get(position);
            }
        } else {
            return mUpdateRepos.get(position);
        }
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title) TextView title;
        @BindView(R.id.version_name) TextView versionName;
        @BindView(R.id.description) TextView description;
        @BindView(R.id.author) TextView author;
        @BindView(R.id.info_layout) LinearLayout infoLayout;
        @BindView(R.id.download) ImageView downloadImage;

        ViewHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }

    }
}
