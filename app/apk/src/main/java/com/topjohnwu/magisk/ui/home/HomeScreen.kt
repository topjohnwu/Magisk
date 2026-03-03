package com.topjohnwu.magisk.ui.home

import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ExperimentalLayoutApi
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.unit.dp
import androidx.core.net.toUri
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.R as CoreR
import top.yukonga.miuix.kmp.basic.Card
import top.yukonga.miuix.kmp.basic.Icon
import top.yukonga.miuix.kmp.basic.LinearProgressIndicator
import top.yukonga.miuix.kmp.basic.MiuixScrollBehavior
import top.yukonga.miuix.kmp.basic.Scaffold
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.basic.TopAppBar
import top.yukonga.miuix.kmp.theme.MiuixTheme

@Composable
fun HomeScreen(viewModel: HomeViewModel) {
    val uiState by viewModel.uiState.collectAsState()
    val context = LocalContext.current
    val scrollBehavior = MiuixScrollBehavior()

    Scaffold(
        topBar = {
            TopAppBar(
                title = stringResource(CoreR.string.section_home),
                scrollBehavior = scrollBehavior
            )
        },
        popupHost = { }
    ) { padding ->
        Column(
            modifier = Modifier
                .nestedScroll(scrollBehavior.nestedScrollConnection)
                .padding(padding)
                .verticalScroll(rememberScrollState())
                .padding(horizontal = 16.dp)
                .padding(bottom = 88.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            if (uiState.isNoticeVisible) {
                NoticeCard(onHide = viewModel::hideNotice)
            }

            MagiskCard(viewModel = viewModel)

            ManagerCard(viewModel = viewModel, uiState = uiState)

            if (Info.env.isActive) {
                TextButton(
                    text = stringResource(CoreR.string.uninstall_magisk_title),
                    onClick = { viewModel.onDeletePressed() },
                    modifier = Modifier.fillMaxWidth()
                )
            }

            SupportCard(onLinkClicked = { viewModel.onLinkPressed(it) })

            DevelopersCard(onLinkClicked = { openLink(context, it) })
        }
    }
}

@Composable
private fun NoticeCard(onHide: () -> Unit) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier.padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = stringResource(CoreR.string.home_notice_content),
                style = MiuixTheme.textStyles.body2,
                modifier = Modifier.weight(1f)
            )
            TextButton(
                text = stringResource(CoreR.string.hide),
                onClick = onHide
            )
        }
    }
}

@Composable
private fun MagiskCard(viewModel: HomeViewModel) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(
                    painter = painterResource(CoreR.drawable.ic_magisk_outline),
                    contentDescription = null,
                    modifier = Modifier.size(32.dp),
                    tint = MiuixTheme.colorScheme.primary
                )
                Spacer(Modifier.width(12.dp))
                Text(
                    text = stringResource(CoreR.string.magisk),
                    style = MiuixTheme.textStyles.headline2,
                    color = MiuixTheme.colorScheme.primary,
                    modifier = Modifier.weight(1f)
                )
                when (viewModel.magiskState) {
                    HomeViewModel.State.OUTDATED -> TextButton(
                        text = stringResource(CoreR.string.update),
                        onClick = { viewModel.onMagiskPressed() }
                    )
                    else -> TextButton(
                        text = stringResource(CoreR.string.install),
                        onClick = { viewModel.onMagiskPressed() }
                    )
                }
            }

            Spacer(Modifier.height(8.dp))

            InfoRow(
                label = stringResource(CoreR.string.home_installed_version),
                value = viewModel.magiskInstalledVersion.ifEmpty {
                    stringResource(CoreR.string.not_available)
                }
            )
            InfoRow(
                label = stringResource(CoreR.string.zygisk),
                value = stringResource(if (Info.isZygiskEnabled) CoreR.string.yes else CoreR.string.no)
            )
            InfoRow(
                label = "Ramdisk",
                value = stringResource(if (Info.ramdisk) CoreR.string.yes else CoreR.string.no)
            )
        }
    }
}

@Composable
private fun ManagerCard(viewModel: HomeViewModel, uiState: HomeViewModel.UiState) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(
                    painter = painterResource(R.drawable.ic_manager),
                    contentDescription = null,
                    modifier = Modifier.size(32.dp),
                    tint = MiuixTheme.colorScheme.primary
                )
                Spacer(Modifier.width(12.dp))
                Text(
                    text = stringResource(CoreR.string.home_app_title),
                    style = MiuixTheme.textStyles.headline2,
                    color = MiuixTheme.colorScheme.primary,
                    modifier = Modifier.weight(1f)
                )
                when (uiState.appState) {
                    HomeViewModel.State.OUTDATED -> TextButton(
                        text = stringResource(CoreR.string.update),
                        onClick = { viewModel.onManagerPressed() }
                    )
                    HomeViewModel.State.UP_TO_DATE -> TextButton(
                        text = stringResource(CoreR.string.install),
                        onClick = { viewModel.onManagerPressed() }
                    )
                    else -> {}
                }
            }

            Spacer(Modifier.height(8.dp))

            InfoRow(
                label = stringResource(CoreR.string.home_latest_version),
                value = uiState.managerRemoteVersion.ifEmpty {
                    stringResource(
                        if (uiState.appState == HomeViewModel.State.LOADING) CoreR.string.loading
                        else CoreR.string.not_available
                    )
                }
            )
            InfoRow(
                label = stringResource(CoreR.string.home_installed_version),
                value = viewModel.managerInstalledVersion
            )
            val context = LocalContext.current
            InfoRow(
                label = stringResource(CoreR.string.home_package),
                value = context.packageName
            )

            if (uiState.managerProgress in 1..99) {
                Spacer(Modifier.height(8.dp))
                LinearProgressIndicator(
                    progress = uiState.managerProgress / 100f,
                    modifier = Modifier.fillMaxWidth()
                )
            }
        }
    }
}

@Composable
private fun InfoRow(label: String, value: String) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 2.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(
            text = label,
            style = MiuixTheme.textStyles.body2,
            color = MiuixTheme.colorScheme.onSurfaceVariantSummary
        )
        Text(
            text = value,
            style = MiuixTheme.textStyles.body2,
        )
    }
}

@Composable
private fun SupportCard(onLinkClicked: (String) -> Unit) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(
                text = stringResource(CoreR.string.home_support_title),
                style = MiuixTheme.textStyles.headline2,
            )
            Spacer(Modifier.height(4.dp))
            Text(
                text = stringResource(CoreR.string.home_support_content),
                style = MiuixTheme.textStyles.body2,
                color = MiuixTheme.colorScheme.onSurfaceVariantSummary
            )
            Spacer(Modifier.height(8.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                LinkChip(
                    icon = CoreR.drawable.ic_patreon,
                    label = stringResource(CoreR.string.patreon),
                    onClick = { onLinkClicked(com.topjohnwu.magisk.core.Const.Url.PATREON_URL) }
                )
                LinkChip(
                    icon = CoreR.drawable.ic_paypal,
                    label = stringResource(CoreR.string.paypal),
                    onClick = { onLinkClicked("https://paypal.me/magiskdonate") }
                )
            }
        }
    }
}

@OptIn(ExperimentalLayoutApi::class)
@Composable
private fun DevelopersCard(onLinkClicked: (String) -> Unit) {
    val developers = listOf(
        DeveloperInfo("topjohnwu", listOf(
            LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/topjohnwu"),
            LinkInfo("GitHub", CoreR.drawable.ic_github, com.topjohnwu.magisk.core.Const.Url.SOURCE_CODE_URL),
        )),
        DeveloperInfo("vvb2060", listOf(
            LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/vvb2060"),
            LinkInfo("GitHub", CoreR.drawable.ic_github, "https://github.com/vvb2060"),
        )),
        DeveloperInfo("yujincheng08", listOf(
            LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/shanasaimoe"),
            LinkInfo("GitHub", CoreR.drawable.ic_github, "https://github.com/yujincheng08"),
            LinkInfo("Sponsor", CoreR.drawable.ic_favorite, "https://github.com/sponsors/yujincheng08"),
        )),
        DeveloperInfo("rikkawww", listOf(
            LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/rikkawww"),
            LinkInfo("GitHub", CoreR.drawable.ic_github, "https://github.com/rikkawww"),
        )),
        DeveloperInfo("canyie", listOf(
            LinkInfo("Twitter", CoreR.drawable.ic_twitter, "https://twitter.com/canyie2977"),
            LinkInfo("GitHub", CoreR.drawable.ic_github, "https://github.com/canyie"),
        )),
    )

    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(
                text = stringResource(CoreR.string.home_follow_title),
                style = MiuixTheme.textStyles.headline2,
            )
            Spacer(Modifier.height(8.dp))
            developers.forEach { dev ->
                DeveloperRow(dev = dev, onLinkClicked = onLinkClicked)
                Spacer(Modifier.height(4.dp))
            }
        }
    }
}

@OptIn(ExperimentalLayoutApi::class)
@Composable
private fun DeveloperRow(dev: DeveloperInfo, onLinkClicked: (String) -> Unit) {
    Column(modifier = Modifier.fillMaxWidth()) {
        Text(
            text = "@${dev.name}",
            style = MiuixTheme.textStyles.body1,
        )
        FlowRow(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            dev.links.forEach { link ->
                LinkChip(
                    icon = link.icon,
                    label = link.label,
                    onClick = { onLinkClicked(link.url) }
                )
            }
        }
    }
}

@Composable
private fun LinkChip(icon: Int, label: String, onClick: () -> Unit) {
    Row(
        modifier = Modifier
            .clickable(onClick = onClick)
            .padding(vertical = 6.dp, horizontal = 4.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(4.dp)
    ) {
        Icon(
            painter = painterResource(icon),
            contentDescription = label,
            modifier = Modifier.size(18.dp),
            tint = MiuixTheme.colorScheme.onSurfaceVariantActions
        )
        Text(
            text = label,
            style = MiuixTheme.textStyles.body2,
            color = MiuixTheme.colorScheme.onSurfaceVariantActions
        )
    }
}

private data class DeveloperInfo(val name: String, val links: List<LinkInfo>)
private data class LinkInfo(val label: String, val icon: Int, val url: String)

private fun openLink(context: Context, url: String) {
    try {
        context.startActivity(Intent(Intent.ACTION_VIEW, url.toUri()).apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        })
    } catch (_: ActivityNotFoundException) { }
}
