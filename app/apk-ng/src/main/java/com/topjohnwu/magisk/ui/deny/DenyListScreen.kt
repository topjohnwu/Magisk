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
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.Sort
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Tune
import androidx.compose.material3.Card
import androidx.compose.material3.Checkbox
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.material3.TriStateCheckbox
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
import com.google.accompanist.drawablepainter.rememberDrawablePainter
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
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

    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.denylist)) },
                navigationIcon = {
                    IconButton(
                        modifier = Modifier.padding(start = 16.dp),
                        onClick = onBack
                    ) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = null,
                            tint = MaterialTheme.colorScheme.onBackground
                        )
                    }
                },
                actions = {
                    Box {
                        IconButton(
                            onClick = { showSortMenu.value = true },
                        ) {
                            Icon(
                                imageVector = Icons.AutoMirrored.Filled.Sort,
                                contentDescription = stringResource(CoreR.string.menu_sort),
                            )
                        }
                        DropdownMenu(
                            expanded = showSortMenu.value,
                            onDismissRequest = { showSortMenu.value = false }
                        ) {
                            val sortOptions = listOf(
                                CoreR.string.sort_by_name to SortBy.NAME,
                                CoreR.string.sort_by_package_name to SortBy.PACKAGE_NAME,
                                CoreR.string.sort_by_install_time to SortBy.INSTALL_TIME,
                                CoreR.string.sort_by_update_time to SortBy.UPDATE_TIME,
                            )
                            sortOptions.forEach { (resId, sort) ->
                                DropdownMenuItem(
                                    text = { Text(stringResource(resId)) },
                                    trailingIcon = if (sortBy == sort) {
                                        { Icon(androidx.compose.material.icons.Icons.Default.Check, contentDescription = null) }
                                    } else null,
                                    onClick = {
                                        viewModel.setSortBy(sort)
                                        showSortMenu.value = false
                                    }
                                )
                            }
                            DropdownMenuItem(
                                text = { Text(stringResource(CoreR.string.sort_reverse)) },
                                trailingIcon = if (sortReverse) {
                                    { Icon(Icons.Default.Check, contentDescription = null) }
                                } else null,
                                onClick = {
                                    viewModel.toggleSortReverse()
                                    showSortMenu.value = false
                                }
                            )
                        }
                    }

                    Box {
                        IconButton(
                            modifier = Modifier.padding(end = 16.dp),
                            onClick = { showFilterMenu.value = true },
                        ) {
                            Icon(
                                imageVector = Icons.Default.Tune,
                                contentDescription = stringResource(CoreR.string.hide_filter_hint),
                            )
                        }
                        DropdownMenu(
                            expanded = showFilterMenu.value,
                            onDismissRequest = { showFilterMenu.value = false }
                        ) {
                            DropdownMenuItem(
                                text = { Text(stringResource(CoreR.string.show_system_app)) },
                                trailingIcon = if (showSystem) {
                                    { Icon(Icons.Default.Check, contentDescription = null) }
                                } else null,
                                onClick = {
                                    viewModel.setShowSystem(!showSystem)
                                    showFilterMenu.value = false
                                }
                            )
                            DropdownMenuItem(
                                text = { Text(stringResource(CoreR.string.show_os_app)) },
                                trailingIcon = if (showOS) {
                                    { Icon(Icons.Default.Check, contentDescription = null) }
                                } else null,
                                onClick = {
                                    if (!showOS && !showSystem) {
                                        viewModel.setShowSystem(true)
                                    }
                                    viewModel.setShowOS(!showOS)
                                    showFilterMenu.value = false
                                }
                            )
                        }
                    }
                },
                scrollBehavior = scrollBehavior
            )
        }
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
                            style = MaterialTheme.typography.titleLarge
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
    OutlinedTextField(
        value = query,
        onValueChange = onQueryChange,
        modifier = modifier,
        label = { Text(stringResource(CoreR.string.hide_filter_hint)) }
    )
}

@Composable
private fun DenyAppCard(app: DenyAppState) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Column {
            if (app.checkedPercent > 0f) {
                LinearProgressIndicator(
                    progress = { app.checkedPercent },
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
                        style = MaterialTheme.typography.bodyLarge,
                    )
                    Text(
                        text = app.info.packageName,
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                Spacer(Modifier.width(8.dp))
                TriStateCheckbox(
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
            style = MaterialTheme.typography.bodyMedium,
            color = if (proc.isEnabled) MaterialTheme.colorScheme.onSurface
                else MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.weight(1f)
        )
        Spacer(Modifier.width(8.dp))
        Checkbox(
            checked = proc.isEnabled,
            onCheckedChange = { proc.toggle() }
        )
    }
}
