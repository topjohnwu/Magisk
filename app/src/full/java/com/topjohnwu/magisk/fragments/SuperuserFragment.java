package com.topjohnwu.magisk.fragments;

import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.ViewBinder;
import com.topjohnwu.magisk.adapters.PolicyAdapter;
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.container.Policy;

import java.util.List;

import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;

public class SuperuserFragment extends BaseFragment {

    public RecyclerView recyclerView;
    public TextView emptyRv;

    private PackageManager pm;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_superuser, container, false);
        ViewBinder.bind(this, view);

        pm = getActivity().getPackageManager();
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

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        ViewBinder.unbind(this);
    }

    private void displayPolicyList() {
        List<Policy> policyList = mm.mDB.getPolicyList(pm);

        if (policyList.size() == 0) {
            emptyRv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            recyclerView.setAdapter(new PolicyAdapter(policyList, mm.mDB, pm));
            emptyRv.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
        }
    }

}
