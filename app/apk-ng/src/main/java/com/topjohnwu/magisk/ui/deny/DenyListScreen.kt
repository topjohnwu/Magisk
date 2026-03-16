package com.topjohnwu.magisk.ui.deny

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.state.ToggleableState
import androidx.compose.ui.unit.dp
import com.topjohnwu.magisk.ui.component.ListPopupDefaults.MenuPositionProvider
import com.topjohnwu.magisk.ui.util.rememberDrawablePainter
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.Checkbox
import top.yukonga.miuix.kmp.basic.CircularProgressIndicator
import top.yukonga.miuix.kmp.basic.DropdownImpl
import top.yukonga.miuix.kmp.basic.Icon
import top.yukonga.miuix.kmp.basic.IconButton
import top.yukonga.miuix.kmp.basic.LinearProgressIndicator
import top.yukonga.miuix.kmp.basic.ListPopupColumn
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.PopupPositionProvider
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.extra.SuperListPopup
import top.yukonga.miuix.kmp.icon.MiuixIcons
import top.yukonga.miuix.kmp.icon.extended.Back
import top.yukonga.miuix.kmp.icon.extended.Sort
import top.yukonga.miuix.kmp.icon.extended.Tune
import top.yukonga.miuix.kmp.theme.MiuixTheme
import com.topjohnwu.magisk.core.R as CoreR

@Composable
fun DenyListScreen(viewModel: DenyListViewModel, onBack: () -> Unit) {
    val loading by viewModel.loading.collectAsState()
    val apps by viewModel.filteredApps.collectAsState()
    val query by viewModel.query.collectAsState()
    val showSystem by viewModel.showSystem.collectAsState()
    val showOS by viewModel.showOS.collectAsState()
    val sortBy by viewModel.sortBy.collectAsState()
    val sortReverse by viewModel.sortReverse.collectAsState()

    val showSortMenu = remember { mutableStateOf(false) }
    val showFilterMenu = remember { mutableStateOf(false) }

    val scrollBehavior = MiuixScrollBehavior()
    Scaffold(
        topBar = {
            TopAppBar(
                title = stringResource(CoreR.string.denylist),
                navigationIcon = {
                    IconButton(
                        modifier = Modifier.padding(start = 16.dp),
                        onClick = onBack
                    ) {
                        Icon(
                            imageVector = MiuixIcons.Back,
                            contentDescription = null,
                            tint = MiuixTheme.colorScheme.onBackground
                        )
                    }
                },
                actions = {
                    Box {
                        IconButton(
                            onClick = { showSortMenu.value = true },
                            holdDownState = showSortMenu.value,
                        ) {
                            Icon(
                                imageVector = MiuixIcons.Sort,
                                contentDescription = stringResource(CoreR.string.menu_sort),
                            )
                        }
                        SuperListPopup(
                            show = showSortMenu,
                            popupPositionProvider = MenuPositionProvider,
                            alignment = PopupPositionProvider.Align.End,
                            onDismissRequest = { showSortMenu.value = false }
                        ) {
                            ListPopupColumn {
                                val sortOptions = listOf(
                                    CoreR.string.sort_by_name to SortBy.NAME,
                                    CoreR.string.sort_by_package_name to SortBy.PACKAGE_NAME,
                                    CoreR.string.sort_by_install_time to SortBy.INSTALL_TIME,
                                    CoreR.string.sort_by_update_time to SortBy.UPDATE_TIME,
                                )
                                val totalSize = sortOptions.size + 1
                                sortOptions.forEachIndexed { index, (resId, sort) ->
                                    DropdownImpl(
                                        text = stringResource(resId),
                                        optionSize = totalSize,
                                        isSelected = sortBy == sort,
                                        index = index,
                                        onSelectedIndexChange = {
                                            viewModel.setSortBy(sort)
                                            showSortMenu.value = false
                                        }
                                    )
                                }
                                DropdownImpl(
                                    text = stringResource(CoreR.string.sort_reverse),
                                    optionSize = totalSize,
                                    isSelected = sortReverse,
                                    index = sortOptions.size,
                                    onSelectedIndexChange = {
                                        viewModel.toggleSortReverse()
                                        showSortMenu.value = false
                                    }
                                )
                            }
                        }
                    }

                    Box {
                        IconButton(
                            modifier = Modifier.padding(end = 16.dp),
                            onClick = { showFilterMenu.value = true },
                            holdDownState = showFilterMenu.value,
                        ) {
                            Icon(
                                imageVector = MiuixIcons.Tune,
                                contentDescription = stringResource(CoreR.string.hide_filter_hint),
                            )
                        }
                        SuperListPopup(
                            show = showFilterMenu,
                            popupPositionProvider = MenuPositionProvider,
                            alignment = PopupPositionProvider.Align.End,
                            onDismissRequest = { showFilterMenu.value = false }
                        ) {
                            ListPopupColumn {
                                DropdownImpl(
                                    text = stringResource(CoreR.string.show_system_app),
                                    optionSize = 2,
                                    isSelected = showSystem,
                                    index = 0,
                                    onSelectedIndexChange = {
                                        viewModel.setShowSystem(!showSystem)
                                        showFilterMenu.value = false
                                    }
                                )
                                DropdownImpl(
                                    text = stringResource(CoreR.string.show_os_app),
                                    optionSize = 2,
                                    isSelected = showOS,
                                    index = 1,
                                    onSelectedIndexChange = {
                                        if (!showOS && !showSystem) {
                                            viewModel.setShowSystem(true)
                                        }
                                        viewModel.setShowOS(!showOS)
                                        showFilterMenu.value = false
                                    }
                                )
                            }
                        }
                    }
                },
                scrollBehavior = scrollBehavior
            )
        },
        popupHost = { }
    ) { padding ->
        Column(modifier = Modifier.fillMaxSize().padding(padding)) {
            SearchInput(
                query = query,
                onQueryChange = viewModel::setQuery,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 12.dp, vertical = 4.dp)
            )

            if (loading) {
                Box(
                    modifier = Modifier.fillMaxSize(),
                    contentAlignment = Alignment.Center
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Text(
                            text = stringResource(CoreR.string.loading),
                            style = MiuixTheme.textStyles.headline2
                        )
                        Spacer(Modifier.height(16.dp))
                        CircularProgressIndicator()
                    }
                }
            } else {
                LazyColumn(
                    modifier = Modifier
                        .fillMaxSize()
                        .nestedScroll(scrollBehavior.nestedScrollConnection)
                        .padding(horizontal = 12.dp),
                    contentPadding = PaddingValues(top = 8.dp, bottom = 88.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    items(
                        items = apps,
                        key = { it.info.packageName }
                    ) { app ->
                        DenyAppCard(app)
                    }
                }
            }
        }
    }
}

@Composable
private fun SearchInput(query: String, onQueryChange: (String) -> Unit, modifier: Modifier = Modifier) {
    top.yukonga.miuix.kmp.basic.TextField(
        value = query,
        onValueChange = onQueryChange,
        modifier = modifier,
        label = stringResource(CoreR.string.hide_filter_hint)
    )
}

@Composable
private fun DenyAppCard(app: DenyAppState) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Column {
            if (app.checkedPercent > 0f) {
                LinearProgressIndicator(
                    progress = app.checkedPercent,
                    modifier = Modifier.fillMaxWidth()
                )
            }

            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { app.isExpanded = !app.isExpanded }
                    .padding(12.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Image(
                    painter = rememberDrawablePainter(app.info.iconImage),
                    contentDescription = app.info.label,
                    modifier = Modifier.size(40.dp)
                )
                Spacer(Modifier.width(12.dp))
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = app.info.label,
                        style = MiuixTheme.textStyles.body1,
                    )
                    Text(
                        text = app.info.packageName,
                        style = MiuixTheme.textStyles.body2,
                        color = MiuixTheme.colorScheme.onSurfaceVariantSummary
                    )
                }
                Spacer(Modifier.width(8.dp))
                Checkbox(
                    state = when {
                        app.itemsChecked == 0 -> ToggleableState.Off
                        app.checkedPercent < 1f -> ToggleableState.Indeterminate
                        else -> ToggleableState.On
                    },
                    onClick = { app.toggleAll() }
                )
            }

            AnimatedVisibility(visible = app.isExpanded) {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(start = 52.dp)
                ) {
                    app.processes.forEach { proc ->
                        ProcessRow(proc)
                    }
                }
            }
        }
    }
}

@Composable
private fun ProcessRow(proc: DenyProcessState) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { proc.toggle() }
            .padding(horizontal = 12.dp, vertical = 6.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = proc.displayName,
            style = MiuixTheme.textStyles.body2,
            color = if (proc.isEnabled) MiuixTheme.colorScheme.onSurface
                else MiuixTheme.colorScheme.onSurfaceVariantSummary,
            modifier = Modifier.weight(1f)
        )
        Spacer(Modifier.width(8.dp))
        Checkbox(
            state = ToggleableState(proc.isEnabled),
            onClick = { proc.toggle() }
        )
    }
}
