package com.topjohnwu.magisk.fragments;

import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.adapters.PolicyAdapter;
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.container.Policy;

import java.util.List;

import butterknife.BindView;

public class SuperuserFragment extends BaseFragment {

    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyRv;

    private PackageManager pm;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_superuser, container, false);
        unbinder = new SuperuserFragment_ViewBinding(this, view);

        pm = requireActivity().getPackageManager();
        return view;
    }

    @Override
    public void onStart() {
        super.onStart();
        requireActivity().setTitle(getString(R.string.superuser));
    }

    @Override
    public void onResume() {
        super.onResume();
        displayPolicyList();
    }

    private void displayPolicyList() {
        List<Policy> policyList = app.mDB.getPolicyList();

        if (policyList.size() == 0) {
            emptyRv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            recyclerView.setAdapter(new PolicyAdapter(policyList, app.mDB, pm));
            emptyRv.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
        }
    }

}
