package com.topjohnwu.magisk;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.support.v7.widget.CardView;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.AnimationHelper;

import org.w3c.dom.Text;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposAdapter extends RecyclerView.Adapter<ReposAdapter.ViewHolder> {

    private final List<Repo> mList;
    private View viewMain;
    private Context context;
    @BindView(R.id.update)
    ImageView updateImage;
    @BindView(R.id.installed)
    ImageView installedImage;
    @BindView(R.id.popup_layout)
    LinearLayout popupLayout;
    @BindView(R.id.author)
    TextView authorText;
    @BindView(R.id.log)
    TextView logText;
    @BindView(R.id.updateStatus) TextView updateStatus;
    @BindView(R.id.installedStatus) TextView installedStatus;
    private boolean isCardExpanded;


    public ReposAdapter(List<Repo> list) {
        this.mList = list;

    }

    private boolean mIsInstalled;




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

        holder.title.setText(repo.getName());
        holder.versionName.setText(repo.getmVersion());
        holder.description.setText(repo.getDescription());
        Log.d("Magisk","ReposAdapter: Setting up info " + repo.getId() + " and " + repo.getDescription() + " and " + repo.getmVersion());
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        if (prefs.contains("repo_isInstalled_" + repo.getId())) {
            mIsInstalled = prefs.getBoolean("repo_isInstalled_" + repo.getId(),false);
            if (mIsInstalled) {
                installedImage.setImageResource(R.drawable.ic_done_black);
                installedStatus.setText(R.string.module_installed);
            }
        }

        isCardExpanded = false;
        AnimationHelper.collapse(popupLayout);

        viewMain.setOnClickListener(new View.OnClickListener() {
            @Override

            public void onClick(View view) {
                if (isCardExpanded) {

                    AnimationHelper.collapse(popupLayout);
                    isCardExpanded = false;
                } else {
                    AnimationHelper.expand(popupLayout);
                    isCardExpanded = true;

                }

//                if (!mIsInstalled) {
//
//                    Utils.DownloadReceiver reciever = new Utils.DownloadReceiver() {
//                        @Override
//                        public void task(File file) {
//                            Log.d("Magisk", "Task firing");
//                            new Utils.FlashZIP(context, repo.getId(), file.toString()).execute();
//                        }
//                    };
//                    String filename = repo.getId().replace(" ", "") + ".zip";
//                    Utils.downloadAndReceive(context, reciever, repo.getmZipUrl(), filename);
//                } else {
//                    Toast.makeText(context,repo.getId() + " is already installed.",Toast.LENGTH_SHORT).show();
//                }
            }
        });


    }




    @Override
    public int getItemCount() {
        return mList.size();
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title) TextView title;

        @BindView(R.id.version_name) TextView versionName;
        @BindView(R.id.description) TextView description;



        public ViewHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }
    }
}