package com.topjohnwu.magisk;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.AnimationHelper;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public abstract class BaseRepoFragment extends Fragment {

    @BindView(R.id.recyclerView)
    RecyclerView recyclerView;
    @BindView(R.id.empty_rv)
    TextView emptyTv;




    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.single_module_fragment, container, false);


        ButterKnife.bind(this, view);

        if (listRepos().size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);

            return view;
        }
        Log.d("Magisk","BaseRepoFragment: ListRepos size is " + listRepos().size());
        recyclerView.setAdapter(new ReposAdapter(listRepos()) {

        });
        return view;
    }


    protected abstract List<Repo> listRepos();

    public class ReposAdapter extends RecyclerView.Adapter<ReposAdapter.ViewHolder> {

        private final List<Repo> mList;
        private View viewMain;
        private Context context;
        @BindView(R.id.update)
        ImageView updateImage;
        @BindView(R.id.installed)
        ImageView installedImage;
//        @BindView(R.id.popup_layout)
//        LinearLayout popupLayout;


        private boolean isCardExpanded;
        private boolean mIsInstalled, mCanUpdate;

        public ReposAdapter(List<Repo> list) {
            this.mList = list;

        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            viewMain = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_repo, parent, false);
            ButterKnife.bind(this, viewMain);
            context = parent.getContext();
            return new ViewHolder(viewMain);
        }

        @Override
        public void onBindViewHolder(final ViewHolder holder, int position) {
            final Repo repo = mList.get(position);
            Log.d("Magisk","ReposAdapter: Trying set up bindview from list pos " + position + " out of a total of " + mList.size() + " and " + repo.getId() );
            if (repo.getId() != null) {
                holder.title.setText(repo.getName());
                holder.versionName.setText(repo.getmVersion());
                holder.description.setText(repo.getDescription());
                Log.d("Magisk", "ReposAdapter: Setting up info " + repo.getId() + " and " + repo.getDescription() + " and " + repo.getmVersion());
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
                if (prefs.contains("repo-isInstalled_" + repo.getId())) {
                    mIsInstalled = prefs.getBoolean("repo-isInstalled_" + repo.getId(), false);
                    if (mIsInstalled) {
                        installedImage.setImageResource(R.drawable.ic_done_black);
                    }
                    mCanUpdate = prefs.getBoolean("repo-isInstalled_" + repo.getId(), false);
                    if (mCanUpdate) {
                        updateImage.setImageResource(R.drawable.ic_system_update_alt_black);
                    }
                }

                isCardExpanded = false;
//                AnimationHelper.collapse(popupLayout);

                viewMain.setOnClickListener(new View.OnClickListener() {
                    @Override

                    public void onClick(View view) {
                        if (isCardExpanded) {
//                            AnimationHelper.expand(popupLayout);
                            isCardExpanded = false;
                        } else {
//                            AnimationHelper.collapse(popupLayout);
                            isCardExpanded = true;

                        }

//                    if (!mIsInstalled | mCanUpdate) {
//
//                        Utils.DownloadReceiver reciever = new Utils.DownloadReceiver() {
//                            @Override
//                            public void task(File file) {
//                                Log.d("Magisk", "Task firing");
//                                new Utils.FlashZIP(context, repo.getId(), file.toString()).execute();
//                            }
//                        };
//                        String filename = repo.getId().replace(" ", "") + ".zip";
//                        Utils.downloadAndReceive(context, reciever, repo.getmZipUrl(), filename);
//                    } else {
//                        Toast.makeText(context,repo.getId() + " is already installed.", Toast.LENGTH_SHORT).show();
//                    }
                    }
                });
            }


        }




        @Override
        public int getItemCount() {
            return mList.size();
        }

        class ViewHolder extends RecyclerView.ViewHolder {

            @BindView(R.id.title) TextView title;
            @BindView(R.id.version_name) TextView versionName;
            @BindView(R.id.description) TextView description;


            public ViewHolder(View itemView) {
                super(itemView);
                ButterKnife.bind(this, itemView);
            }
        }
    }
}
