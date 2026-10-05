package com.topjohnwu.magisk.ui.superuser

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.Sort
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Search
import androidx.compose.material.icons.filled.Tune
import androidx.compose.material3.Checkbox
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SearchBarDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.material3.TextFieldDefaults
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.DpOffset
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.topjohnwu.magisk.ui.component.verticalScrollbar
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SuperuserGrantScreen(
    viewModel: SuperuserGrantViewModel,
    onBack: () -> Unit,
    modifier: Modifier = Modifier,
) {

    val loading by viewModel.loading.collectAsStateWithLifecycle()
    val apps by viewModel.filteredApps.collectAsStateWithLifecycle()
    val query by viewModel.query.collectAsStateWithLifecycle()
    val showSystem by viewModel.showSystem.collectAsStateWithLifecycle()
    val showOS by viewModel.showOS.collectAsStateWithLifecycle()
    val sortBy by viewModel.sortBy.collectAsStateWithLifecycle()
    val sortReverse by viewModel.sortReverse.collectAsStateWithLifecycle()

    var showSortMenu by remember { mutableStateOf(false) }
    var showFilterMenu by remember { mutableStateOf(false) }

    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    Scaffold(
        modifier = modifier,
        topBar = {
            TopAppBar(
                title = { Text(stringResource(CoreR.string.superuser_grant)) },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = stringResource(CoreR.string.back),
                        )
                    }
                },
                actions = {
                    Box {
                        IconButton(onClick = { showSortMenu = true }) {
                            Icon(
                                imageVector = Icons.AutoMirrored.Filled.Sort,
                                contentDescription = stringResource(CoreR.string.menu_sort),
                            )
                        }
                        DropdownMenu(
                            expanded = showSortMenu,
                            onDismissRequest = { showSortMenu = false }
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
                                        { Icon(Icons.Default.Check, contentDescription = null) }
                                    } else null,
                                    onClick = {
                                        viewModel.setSortBy(sort)
                                        showSortMenu = false
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
                                    showSortMenu = false
                                }
                            )
                        }
                    }

                    Box {
                        IconButton(onClick = { showFilterMenu = true }) {
                            Icon(
                                imageVector = Icons.Default.Tune,
                                contentDescription = stringResource(CoreR.string.hide_filter_hint),
                            )
                        }
                        DropdownMenu(
                            expanded = showFilterMenu,
                            onDismissRequest = { showFilterMenu = false },
                            offset = DpOffset(x = (-8).dp, y = 0.dp),
                        ) {
                            DropdownMenuItem(
                                text = { Text(stringResource(CoreR.string.show_system_app)) },
                                trailingIcon = if (showSystem) {
                                    { Icon(Icons.Default.Check, contentDescription = null) }
                                } else null,
                                onClick = {
                                    viewModel.setShowSystem(!showSystem)
                                    showFilterMenu = false
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
                                    showFilterMenu = false
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
                    .padding(horizontal = 16.dp, vertical = 6.dp)
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
                val listState = rememberLazyListState()
                LazyColumn(
                    state = listState,
                    modifier = Modifier
                        .fillMaxSize()
                        .nestedScroll(scrollBehavior.nestedScrollConnection)
                        .verticalScrollbar(listState, contentPadding = PaddingValues(vertical = 8.dp)),
                    contentPadding = PaddingValues(horizontal = 12.dp, vertical = 8.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    items(
                        items = apps,
                        key = { it.info.packageName },
                        contentType = { "SuGrantAppCard" }
                    ) { app ->
                        SuGrantAppCard(
                            app = app,
                            onToggle = { viewModel.toggleGrant(app) }
                        )
                    }
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun SearchInput(
    query: String,
    onQueryChange: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    TextField(
        value = query,
        onValueChange = onQueryChange,
        modifier = modifier,
        placeholder = { Text(stringResource(CoreR.string.hide_filter_hint)) },
        leadingIcon = {
            Icon(
                imageVector = Icons.Default.Search,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant
            )
        },
        trailingIcon = if (query.isNotEmpty()) {
            {
                IconButton(onClick = { onQueryChange("") }) {
                    Icon(
                        imageVector = Icons.Default.Close,
                        contentDescription = stringResource(CoreR.string.close),
                        tint = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        } else null,
        shape = SearchBarDefaults.inputFieldShape,
        singleLine = true,
        colors = TextFieldDefaults.colors(
            focusedContainerColor = MaterialTheme.colorScheme.surfaceContainerHigh,
            unfocusedContainerColor = MaterialTheme.colorScheme.surfaceContainerHigh,
            focusedIndicatorColor = Color.Transparent,
            unfocusedIndicatorColor = Color.Transparent,
            disabledIndicatorColor = Color.Transparent,
        )
    )
}

@Composable
private fun SuGrantAppCard(
    app: SuGrantAppState,
    onToggle: () -> Unit,
    modifier: Modifier = Modifier
) {
    SuperuserAppCard(
        icon = app.info.icon,
        title = app.info.label,
        packageName = app.info.packageName,
        isSharedUid = app.info.isSharedUid,
        checked = app.isGranted,
        onCheckedChange = { onToggle() },
        contentDescription = app.info.label,
        modifier = modifier,
        trailingContent = {
            Checkbox(
                checked = app.isGranted,
                onCheckedChange = null
            )
        }
    )
}
