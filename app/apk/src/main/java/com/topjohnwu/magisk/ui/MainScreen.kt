package com.topjohnwu.magisk.ui

import android.net.Uri
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.pager.HorizontalPager
import androidx.compose.foundation.pager.rememberPagerState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.snapshotFlow
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.res.vectorResource
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.lifecycle.viewmodel.compose.viewModel
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.AsyncLoadViewModel
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.ui.deny.DenyListScreen
import com.topjohnwu.magisk.ui.deny.DenyListViewModel
import com.topjohnwu.magisk.ui.flash.FlashScreen
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import com.topjohnwu.magisk.ui.home.HomeScreen
import com.topjohnwu.magisk.ui.home.HomeViewModel
import com.topjohnwu.magisk.ui.install.InstallScreen
import com.topjohnwu.magisk.ui.install.InstallViewModel
import com.topjohnwu.magisk.ui.log.LogScreen
import com.topjohnwu.magisk.ui.log.LogViewModel
import com.topjohnwu.magisk.ui.module.ActionScreen
import com.topjohnwu.magisk.ui.module.ActionViewModel
import com.topjohnwu.magisk.ui.module.ModuleScreen
import com.topjohnwu.magisk.ui.module.ModuleViewModel
import com.topjohnwu.magisk.ui.navigation.CollectNavEvents
import com.topjohnwu.magisk.ui.navigation.LocalNavigator
import com.topjohnwu.magisk.ui.navigation.Navigator
import com.topjohnwu.magisk.ui.navigation.ObserveViewEvents
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.ui.settings.SettingsScreen
import com.topjohnwu.magisk.ui.settings.SettingsViewModel
import com.topjohnwu.magisk.ui.superuser.SuperuserScreen
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import kotlinx.coroutines.launch
import top.yukonga.miuix.kmp.basic.Icon
import top.yukonga.miuix.kmp.basic.NavigationBar
import top.yukonga.miuix.kmp.basic.NavigationBarItem
import top.yukonga.miuix.kmp.basic.NavigationItem
import com.topjohnwu.magisk.core.R as CoreR

enum class Tab(val titleRes: Int, val iconRes: Int) {
    HOME(CoreR.string.section_home, R.drawable.ic_home_outlined_md2),
    SUPERUSER(CoreR.string.superuser, R.drawable.ic_superuser_outlined_md2),
    LOG(CoreR.string.logs, R.drawable.ic_bug_outlined_md2),
    MODULES(CoreR.string.modules, R.drawable.ic_module_outlined_md2),
    SETTINGS(CoreR.string.settings, R.drawable.ic_settings_outlined_md2);
}

@Composable
fun MainScreen(initialTab: Int = 0) {
    val navigator = LocalNavigator.current
    val pagerState = rememberPagerState(initialPage = initialTab, pageCount = { Tab.entries.size })
    val scope = rememberCoroutineScope()

    val items = Tab.entries.map { tab ->
        NavigationItem(
            label = stringResource(tab.titleRes),
            icon = ImageVector.vectorResource(tab.iconRes),
        )
    }

    Column(modifier = Modifier.fillMaxSize()) {
        HorizontalPager(
            state = pagerState,
            modifier = Modifier.weight(1f),
            beyondViewportPageCount = Tab.entries.size - 1,
            userScrollEnabled = true,
        ) { page ->
            when (Tab.entries[page]) {
                Tab.HOME -> {
                    val vm: HomeViewModel = viewModel(factory = VMFactory)
                    ObserveViewEvents(vm)
                    CollectNavEvents(vm, navigator)
                    HomeScreen(vm)
                }
                Tab.SUPERUSER -> {
                    val vm: SuperuserViewModel = viewModel(factory = VMFactory)
                    ObserveViewEvents(vm)
                    SuperuserScreen(vm)
                }
                Tab.LOG -> {
                    val vm: LogViewModel = viewModel(factory = VMFactory)
                    ObserveViewEvents(vm)
                    LogScreen(vm)
                }
                Tab.MODULES -> {
                    val vm: ModuleViewModel = viewModel(factory = VMFactory)
                    ObserveViewEvents(vm)
                    CollectNavEvents(vm, navigator)
                    ModuleScreen(vm)
                }
                Tab.SETTINGS -> {
                    val vm: SettingsViewModel = viewModel(factory = VMFactory)
                    ObserveViewEvents(vm)
                    CollectNavEvents(vm, navigator)
                    SettingsScreen(vm)
                }
            }
        }

        NavigationBar {
            items.forEachIndexed { index, item ->
                val tab = Tab.entries[index]
                val enabled = when (tab) {
                    Tab.SUPERUSER -> Info.showSuperUser
                    Tab.MODULES -> Info.env.isActive && LocalModule.loaded()
                    else -> true
                }
                NavigationBarItem(
                    modifier = Modifier.weight(1f),
                    icon = item.icon,
                    label = item.label,
                    selected = pagerState.currentPage == index,
                    enabled = enabled,
                    onClick = {
                        scope.launch { pagerState.animateScrollToPage(index) }
                    }
                )
            }
        }
    }
}
