package com.topjohnwu.magisk;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposAdapter extends RecyclerView.Adapter<ReposAdapter.ViewHolder> {

    private final List<Repo> mList;
    private View view;
    private Context context;

    public ReposAdapter(List<Repo> list) {
        this.mList = list;

    }



    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        view = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_repo, parent, false);
        context = parent.getContext();

        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        final Repo repo = mList.get(position);

        holder.title.setText(repo.getName());
        holder.versionName.setText(repo.getVersion());
        holder.description.setText(repo.getDescription());
        view.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                Utils.DownloadReceiver reciever = new Utils.DownloadReceiver() {
                    @Override
                    public void task(File file) {
                        Log.d("Magisk","Task firing");
                        new Utils.FlashZIP(context,repo.getName(),file.toString()).execute();
                    }
                };
                String filename = repo.getName().replace(" ","") + ".zip";
                Utils.downloadAndReceive(context,reciever,repo.getZipUrl(),filename);

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