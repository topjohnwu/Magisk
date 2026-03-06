package com.topjohnwu.magisk.ui

import android.net.Uri
import android.os.Build
import androidx.compose.animation.AnimatedContent
import androidx.compose.animation.AnimatedContentTransitionScope
import androidx.compose.animation.EnterTransition
import androidx.compose.animation.ExitTransition
import androidx.compose.animation.SizeTransform
import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.core.FastOutLinearInEasing
import androidx.compose.animation.core.FastOutSlowInEasing
import androidx.compose.animation.core.LinearOutSlowInEasing
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.scaleIn
import androidx.compose.animation.scaleOut
import androidx.compose.animation.slideInHorizontally
import androidx.compose.animation.slideInVertically
import androidx.compose.animation.slideOutHorizontally
import androidx.compose.animation.slideOutVertically
import androidx.compose.animation.togetherWith
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.ArrowBack
import androidx.compose.material.icons.rounded.Block
import androidx.compose.material.icons.rounded.CheckCircle
import androidx.compose.material.icons.rounded.Download
import androidx.compose.material.icons.rounded.Error
import androidx.compose.material.icons.rounded.Extension
import androidx.compose.material.icons.rounded.HistoryEdu
import androidx.compose.material.icons.rounded.Home
import androidx.compose.material.icons.rounded.Info
import androidx.compose.material.icons.rounded.Palette
import androidx.compose.material.icons.rounded.PlayCircle
import androidx.compose.material.icons.rounded.PowerSettingsNew
import androidx.compose.material.icons.rounded.Settings
import androidx.compose.material.icons.rounded.Shield
import androidx.compose.material.icons.rounded.Terminal
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarData
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.colorResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.navigation.NavType
import androidx.navigation.NavGraph.Companion.findStartDestination
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import androidx.navigation.navArgument
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.ui.deny.DenyListScreen
import com.topjohnwu.magisk.ui.flash.FlashScreen
import com.topjohnwu.magisk.ui.home.HomeScreen
import com.topjohnwu.magisk.ui.install.InstallScreen
import com.topjohnwu.magisk.ui.log.LogsScreen
import com.topjohnwu.magisk.ui.module.ModuleActionScreen
import com.topjohnwu.magisk.ui.module.ModuleScreen
import com.topjohnwu.magisk.ui.settings.SettingsScreen
import com.topjohnwu.magisk.ui.settings.ThemeScreen
import com.topjohnwu.magisk.ui.superuser.SuperuserLogsScreen
import com.topjohnwu.magisk.ui.superuser.SuperuserScreen
import com.topjohnwu.magisk.ui.theme.magiskComposeColorScheme
import kotlin.math.abs
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MagiskAppContainer(
    useDynamicColor: Boolean = Build.VERSION.SDK_INT >= Build.VERSION_CODES.S,
    darkTheme: Boolean = isSystemInDarkTheme(),
    openSection: String? = null
) {
    val context = LocalContext.current
    val colorScheme = magiskComposeColorScheme(
        useDynamicColor = useDynamicColor,
        darkTheme = darkTheme
    )

    MaterialTheme(colorScheme = colorScheme) {
        val navController = rememberNavController()
        val snackbarHostState = remember { SnackbarHostState() }
        var homeRebootRequestToken by remember { mutableStateOf(0) }
        var homeRebootConsumedToken by remember { mutableStateOf(0) }

        val rootDestinations = remember {
            buildList {
                add(AppDestination.Home)
                if (Info.isRooted && Info.env.isActive) add(AppDestination.Modules)
                if (Info.showSuperUser) add(AppDestination.Superuser)
                add(AppDestination.Logs)
            }
        }
        val rootRoutes = remember(rootDestinations) { rootDestinations.map { it.route }.toSet() }
        val rootRouteOrder = remember(rootDestinations) {
            rootDestinations.mapIndexed { index, destination -> destination.route to index }.toMap()
        }
        val backStackEntry by navController.currentBackStackEntryAsState()
        val currentRoute = backStackEntry?.destination?.route ?: AppRoute.Home
        val currentRoot =
            rootDestinations.firstOrNull { it.route == currentRoute } ?: AppDestination.Home
        val isRootRoute = currentRoute in rootRoutes
        val reserveBottomForLogs = isRootRoute && currentRoute == AppRoute.Logs
        val moduleActionNameArg = backStackEntry?.arguments?.getString("name")
        var flashTitleOverride by remember { mutableStateOf<String?>(null) }
        var flashSubtitleOverride by remember { mutableStateOf<String?>(null) }
        var flashProcessStateOverride by remember { mutableStateOf(RouteProcessTopBarState()) }
        var moduleRunTitleOverride by remember { mutableStateOf<String?>(null) }
        var moduleRunSubtitleOverride by remember { mutableStateOf<String?>(null) }
        var moduleRunProcessStateOverride by remember { mutableStateOf(RouteProcessTopBarState()) }
        val backEnabled = when (currentRoute) {
            AppRoute.FlashPattern -> !flashProcessStateOverride.running
            AppRoute.ModuleActionPattern -> !moduleRunProcessStateOverride.running
            else -> true
        }

        LaunchedEffect(openSection) {
            val route = when (openSection) {
                Const.Nav.SUPERUSER -> AppRoute.Superuser
                Const.Nav.MODULES -> AppRoute.Modules
                Const.Nav.SETTINGS -> AppRoute.Settings
                else -> null
            } ?: return@LaunchedEffect
            if (route == currentRoute) return@LaunchedEffect

            navController.navigate(route) {
                launchSingleTop = true
                restoreState = true
            }
        }

        LaunchedEffect(currentRoute) {
            if (currentRoute != AppRoute.FlashPattern) {
                flashTitleOverride = null
                flashSubtitleOverride = null
                flashProcessStateOverride = RouteProcessTopBarState()
            }
            if (currentRoute != AppRoute.ModuleActionPattern) {
                moduleRunTitleOverride = null
                moduleRunSubtitleOverride = null
                moduleRunProcessStateOverride = RouteProcessTopBarState()
            }
        }

        Scaffold(
            modifier = Modifier.fillMaxSize(),
            topBar = {
                MagiskFloatingTopBar(
                    currentRoute = currentRoute,
                    currentRoot = currentRoot,
                    isRootRoute = isRootRoute,
                    moduleActionNameArg = moduleActionNameArg,
                    flashTitleOverride = flashTitleOverride,
                    flashSubtitleOverride = flashSubtitleOverride,
                    flashProcessStateOverride = flashProcessStateOverride,
                    moduleRunTitleOverride = moduleRunTitleOverride,
                    moduleRunSubtitleOverride = moduleRunSubtitleOverride,
                    moduleRunProcessStateOverride = moduleRunProcessStateOverride,
                    backEnabled = backEnabled,
                    onBack = { navController.popBackStack() },
                    onHomePower = { homeRebootRequestToken++ },
                    onOpenSettings = { navController.navigate(AppRoute.Settings) }
                )
            },
            snackbarHost = {
                SnackbarHost(hostState = snackbarHostState) { data -> ExpressiveSnackbar(data) }
            }
        ) { paddingValues ->
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(top = paddingValues.calculateTopPadding())
            ) {

                NavHost(
                    navController = navController,
                    startDestination = AppRoute.Home,
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(bottom = if (reserveBottomForLogs) LOGS_RESERVED_BOTTOM_SPACE else 0.dp),
                    enterTransition = {
                        val initialRoute = initialState.destination.route
                        val targetRoute = targetState.destination.route
                        routeEnterTransition(
                            direction = routeDirection(
                                initialRoute,
                                targetRoute,
                                rootRouteOrder
                            ),
                            rootTabDistance = rootRouteDistance(
                                initialRoute,
                                targetRoute,
                                rootRouteOrder
                            )
                        )
                    },
                    exitTransition = {
                        val initialRoute = initialState.destination.route
                        val targetRoute = targetState.destination.route
                        routeExitTransition(
                            direction = routeDirection(
                                initialRoute,
                                targetRoute,
                                rootRouteOrder
                            ),
                            rootTabDistance = rootRouteDistance(
                                initialRoute,
                                targetRoute,
                                rootRouteOrder
                            )
                        )
                    },
                    popEnterTransition = {
                        val initialRoute = initialState.destination.route
                        val targetRoute = targetState.destination.route
                        routeEnterTransition(
                            direction = routeDirection(
                                initialRoute,
                                targetRoute,
                                rootRouteOrder
                            ),
                            rootTabDistance = rootRouteDistance(
                                initialRoute,
                                targetRoute,
                                rootRouteOrder
                            )
                        )
                    },
                    popExitTransition = {
                        val initialRoute = initialState.destination.route
                        val targetRoute = targetState.destination.route
                        routeExitTransition(
                            direction = routeDirection(
                                initialRoute,
                                targetRoute,
                                rootRouteOrder
                            ),
                            rootTabDistance = rootRouteDistance(
                                initialRoute,
                                targetRoute,
                                rootRouteOrder
                            )
                        )
                    }
                ) {
                    composable(AppRoute.Home) {
                        HomeScreen(
                            rebootRequestToken = if (homeRebootRequestToken > homeRebootConsumedToken) {
                                homeRebootRequestToken
                            } else {
                                0
                            },
                            onRebootTokenConsumed = {
                                homeRebootConsumedToken = homeRebootRequestToken
                            },
                            onOpenInstall = { navController.navigate(AppRoute.Install) },
                            onOpenUninstall = {
                                navController.navigate(
                                    AppRoute.flash(
                                        Const.Value.UNINSTALL,
                                        null
                                    )
                                )
                            }
                        )
                    }
                    composable(AppRoute.Modules) {
                        ModuleScreen(
                            onInstallZip = { uri ->
                                navController.navigate(
                                    AppRoute.flash(Const.Value.FLASH_ZIP, uri.toString())
                                )
                            },
                            onRunAction = { id, name ->
                                navController.navigate(
                                    AppRoute.moduleAction(
                                        id,
                                        name
                                    )
                                )
                            })
                    }
                    composable(AppRoute.Superuser) {
                        SuperuserScreen(onOpenLogs = {
                            navController.navigate(
                                AppRoute.History
                            )
                        })
                    }
                    composable(AppRoute.Logs) { LogsScreen() }

                    composable(AppRoute.Settings) {
                        SettingsScreen(
                            onOpenDenyList = { navController.navigate(AppRoute.DenyList) },
                            onOpenTheme = { navController.navigate(AppRoute.Theme) }
                        )
                    }
                    composable(AppRoute.Theme) {
                        ThemeScreen(
                            onThemeChanged = { (context as? android.app.Activity)?.recreate() }
                        )
                    }
                    composable(AppRoute.History) { SuperuserLogsScreen() }
                    composable(AppRoute.DenyList) { DenyListScreen(onBack = { navController.popBackStack() }) }
                    composable(AppRoute.Install) {
                        InstallScreen(onStartFlash = { action, uri ->
                            navController.navigate(
                                AppRoute.flash(action, uri?.toString())
                            )
                        })
                    }
                    composable(
                        route = AppRoute.FlashPattern,
                        arguments = listOf(
                            navArgument("action") { type = NavType.StringType },
                            navArgument("uri") {
                                type = NavType.StringType; nullable = true; defaultValue = null
                            })
                    ) { entry ->
                        val action = entry.arguments?.getString("action").orEmpty()
                        val uriArg = entry.arguments?.getString("uri")
                        FlashScreen(
                            action = action,
                            uriArg = uriArg,
                            onTitleStateChange = { title, subtitle, processState ->
                                flashTitleOverride = title
                                flashSubtitleOverride = subtitle
                                flashProcessStateOverride = processState
                            },
                            onBack = { navController.popBackStack() }
                        )
                    }
                    composable(
                        route = AppRoute.ModuleActionPattern,
                        arguments = listOf(
                            navArgument("id") { type = NavType.StringType },
                            navArgument("name") { type = NavType.StringType; defaultValue = "" })
                    ) { entry ->
                        val id = entry.arguments?.getString("id").orEmpty()
                        val name = entry.arguments?.getString("name").orEmpty()
                        val safeName = runCatching { Uri.decode(name) }.getOrDefault(name)
                        ModuleActionScreen(
                            actionId = id,
                            actionName = safeName,
                            onTitleStateChange = { title, subtitle, processState ->
                                moduleRunTitleOverride = title
                                moduleRunSubtitleOverride = subtitle
                                moduleRunProcessStateOverride = processState
                            },
                            onBack = { navController.popBackStack() }
                        )
                    }
                }

                if (isRootRoute) {
                    Box(modifier = Modifier.align(Alignment.BottomCenter)) {
                        MagiskFloatingBottomBar(
                            destinations = rootDestinations,
                            currentRoute = currentRoute,
                            floating = true,
                            onNavigate = { route ->
                                navController.navigate(route) {
                                    popUpTo(navController.graph.findStartDestination().id) {
                                        saveState = true
                                    }
                                    launchSingleTop = true
                                    restoreState = true
                                }
                            })
                    }
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun MagiskFloatingTopBar(
    currentRoute: String,
    currentRoot: AppDestination,
    isRootRoute: Boolean,
    moduleActionNameArg: String?,
    flashTitleOverride: String?,
    flashSubtitleOverride: String?,
    flashProcessStateOverride: RouteProcessTopBarState,
    moduleRunTitleOverride: String?,
    moduleRunSubtitleOverride: String?,
    moduleRunProcessStateOverride: RouteProcessTopBarState,
    backEnabled: Boolean,
    onBack: () -> Unit,
    onHomePower: () -> Unit,
    onOpenSettings: () -> Unit
) {
    val moduleActionName = remember(moduleActionNameArg) {
        moduleActionNameArg?.let { runCatching { Uri.decode(it) }.getOrDefault(it) }
    }

    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .statusBarsPadding()
            .height(72.dp),
        shape = RoundedCornerShape(bottomStart = 24.dp, bottomEnd = 24.dp),
        color = MaterialTheme.colorScheme.surfaceContainerHighest,
        tonalElevation = 4.dp,
        shadowElevation = 0.dp
    ) {
        Row(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            if (!isRootRoute) {
                IconButton(
                    enabled = backEnabled,
                    onClick = onBack,
                    modifier = Modifier.background(
                        MaterialTheme.colorScheme.surfaceVariant.copy(
                            alpha = 0.3f
                        ), CircleShape
                    )
                ) {
                    Icon(Icons.AutoMirrored.Rounded.ArrowBack, contentDescription = null)
                }
                Spacer(Modifier.width(12.dp))
            } else {
                Spacer(Modifier.width(8.dp))
            }

            Row(
                modifier = Modifier.weight(1f),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                val displayIcon = when (currentRoute) {
                    AppRoute.DenyList -> Icons.Rounded.Block
                    AppRoute.Install -> Icons.Rounded.Download
                    AppRoute.Theme -> Icons.Rounded.Palette
                    AppRoute.FlashPattern -> Icons.Rounded.Terminal
                    AppRoute.ModuleActionPattern -> Icons.Rounded.PlayCircle
                    AppRoute.History -> Icons.Rounded.HistoryEdu
                    AppRoute.Settings -> Icons.Rounded.Settings
                    else -> currentRoot.selectedIcon
                }
                val processState = when (currentRoute) {
                    AppRoute.FlashPattern -> flashProcessStateOverride
                    AppRoute.ModuleActionPattern -> moduleRunProcessStateOverride
                    else -> RouteProcessTopBarState()
                }

                AnimatedContent(
                    targetState = processState,
                    transitionSpec = {
                        (
                            (fadeIn(
                                animationSpec = tween(
                                    durationMillis = 160,
                                    delayMillis = 60,
                                    easing = LinearOutSlowInEasing
                                )
                            ) + scaleIn(
                                initialScale = 0.9f,
                                animationSpec = tween(
                                    durationMillis = 260,
                                    easing = FastOutSlowInEasing
                                )
                            )) togetherWith
                                (fadeOut(
                                    animationSpec = tween(
                                        durationMillis = 100,
                                        easing = FastOutLinearInEasing
                                    )
                                ) + scaleOut(
                                    targetScale = 0.96f,
                                    animationSpec = tween(
                                        durationMillis = 120,
                                        easing = FastOutLinearInEasing
                                    )
                                ))
                            ).using(SizeTransform(clip = false))
                    },
                    label = "topBarStatus"
                ) { status ->
                    if (status.running) {
                        CircularProgressIndicator(
                            modifier = Modifier.size(24.dp),
                            strokeWidth = 3.dp,
                            color = MaterialTheme.colorScheme.primary,
                            trackColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.1f)
                        )
                    } else if (status.hasResult) {
                        Surface(
                            color = if (status.success) MaterialTheme.colorScheme.primaryContainer else MaterialTheme.colorScheme.errorContainer,
                            shape = RoundedCornerShape(16.dp),
                            modifier = Modifier.size(40.dp)
                        ) {
                            Icon(
                                imageVector = if (status.success) Icons.Rounded.CheckCircle else Icons.Rounded.Error,
                                contentDescription = null,
                                modifier = Modifier.padding(8.dp),
                                tint = if (status.success) MaterialTheme.colorScheme.onPrimaryContainer else MaterialTheme.colorScheme.onErrorContainer
                            )
                        }
                    } else {
                        Surface(
                            color = MaterialTheme.colorScheme.primaryContainer,
                            shape = RoundedCornerShape(16.dp),
                            modifier = Modifier.size(40.dp)
                        ) {
                            Icon(
                                imageVector = displayIcon,
                                contentDescription = null,
                                modifier = Modifier.padding(8.dp),
                                tint = MaterialTheme.colorScheme.onPrimaryContainer
                            )
                        }
                    }
                }

                val baseTitleText = when (currentRoute) {
                    AppRoute.DenyList -> stringResource(id = CoreR.string.denylist)
                    AppRoute.Install -> stringResource(id = CoreR.string.install)
                    AppRoute.Theme -> stringResource(id = CoreR.string.section_theme)
                    AppRoute.FlashPattern -> stringResource(id = CoreR.string.flash_screen_title)
                    AppRoute.ModuleActionPattern -> stringResource(id = CoreR.string.module_action)
                    AppRoute.History -> stringResource(id = CoreR.string.superuser_logs)
                    AppRoute.Settings -> stringResource(id = CoreR.string.settings)
                    else -> stringResource(id = currentRoot.labelRes)
                }
                val resolvedTitleText = when (currentRoute) {
                    AppRoute.FlashPattern -> flashTitleOverride ?: baseTitleText
                    AppRoute.ModuleActionPattern -> moduleRunTitleOverride ?: baseTitleText
                    else -> baseTitleText
                }
                val titleText =
                    if (currentRoute == AppRoute.FlashPattern || currentRoute == AppRoute.ModuleActionPattern) {
                        resolvedTitleText
                    } else {
                        resolvedTitleText.uppercase()
                    }
                val subtitleText = when (currentRoute) {
                    AppRoute.FlashPattern -> flashSubtitleOverride
                    AppRoute.ModuleActionPattern -> moduleRunSubtitleOverride ?: moduleActionName
                    else -> null
                }

                AnimatedContent(
                    targetState = TopBarLabelState(
                        title = titleText,
                        subtitle = subtitleText
                    ),
                    transitionSpec = {
                        (
                            (fadeIn(
                                animationSpec = tween(
                                    durationMillis = 170,
                                    delayMillis = 40,
                                    easing = LinearOutSlowInEasing
                                )
                            ) + slideInVertically(
                                initialOffsetY = { it / 6 },
                                animationSpec = tween(
                                    durationMillis = 220,
                                    easing = FastOutSlowInEasing
                                )
                            )) togetherWith
                                (fadeOut(
                                    animationSpec = tween(
                                        durationMillis = 110,
                                        easing = FastOutLinearInEasing
                                    )
                                ) + slideOutVertically(
                                    targetOffsetY = { -it / 8 },
                                    animationSpec = tween(
                                        durationMillis = 140,
                                        easing = FastOutLinearInEasing
                                    )
                                ))
                            ).using(SizeTransform(clip = false))
                    },
                    label = "topBarLabel"
                ) { label ->
                    Column(
                        verticalArrangement = Arrangement.Center
                    ) {
                        Text(
                            text = label.title,
                            style = if (label.subtitle != null) MaterialTheme.typography.titleMedium else MaterialTheme.typography.titleLarge,
                            fontWeight = FontWeight.Black,
                            letterSpacing = 1.sp,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                        if (label.subtitle != null) {
                            Text(
                                text = label.subtitle,
                                style = MaterialTheme.typography.labelMedium,
                                color = MaterialTheme.colorScheme.onSurfaceVariant,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis
                            )
                        }
                    }
                }
            }

            if (currentRoute == AppRoute.Home) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    IconButton(
                        onClick = onHomePower,
                        modifier = Modifier.background(
                            MaterialTheme.colorScheme.surfaceVariant.copy(
                                alpha = 0.3f
                            ), CircleShape
                        )
                    ) {
                        Icon(
                            Icons.Rounded.PowerSettingsNew,
                            contentDescription = stringResource(id = CoreR.string.reboot)
                        )
                    }
                    IconButton(
                        onClick = onOpenSettings,
                        modifier = Modifier.background(
                            MaterialTheme.colorScheme.surfaceVariant.copy(
                                alpha = 0.3f
                            ), CircleShape
                        )
                    ) {
                        Icon(
                            Icons.Rounded.Settings,
                            contentDescription = stringResource(id = CoreR.string.settings)
                        )
                    }
                }
            } else {
                Spacer(Modifier.width(8.dp))
            }
        }
    }
}

@Composable
private fun ExpressiveSnackbar(snackbarData: SnackbarData) {
    Surface(
        modifier = Modifier
            .padding(16.dp)
            .padding(bottom = 120.dp),
        shape = RoundedCornerShape(24.dp),
        color = MaterialTheme.colorScheme.surfaceContainerHighest,
        tonalElevation = 6.dp,
        shadowElevation = 8.dp
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 20.dp, vertical = 16.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Surface(
                color = MaterialTheme.colorScheme.primaryContainer,
                shape = CircleShape,
                modifier = Modifier.size(32.dp)
            ) {
                Icon(
                    Icons.Rounded.Info,
                    null,
                    tint = MaterialTheme.colorScheme.onPrimaryContainer,
                    modifier = Modifier.padding(6.dp)
                )
            }
            Text(
                text = snackbarData.visuals.message,
                style = MaterialTheme.typography.bodyMedium,
                fontWeight = FontWeight.Bold,
                color = MaterialTheme.colorScheme.onSurface,
                modifier = Modifier.weight(1f)
            )
            if (snackbarData.visuals.actionLabel != null) {
                Button(
                    onClick = { snackbarData.performAction() },
                    shape = RoundedCornerShape(12.dp),
                    contentPadding = PaddingValues(horizontal = 16.dp, vertical = 8.dp)
                ) {
                    Text(
                        snackbarData.visuals.actionLabel!!,
                        fontWeight = FontWeight.Black,
                        fontSize = 12.sp
                    )
                }
            }
        }
    }
}

@Composable
private fun MagiskFloatingBottomBar(
    destinations: List<AppDestination>,
    currentRoute: String,
    floating: Boolean,
    onNavigate: (String) -> Unit
) {
    val barModifier = if (floating) {
        Modifier
            .padding(horizontal = 20.dp, vertical = 32.dp)
            .navigationBarsPadding()
            .height(84.dp)
    } else {
        Modifier
            .fillMaxWidth()
            .navigationBarsPadding()
            .height(72.dp)
    }

    Surface(
        modifier = barModifier,
        shape = if (floating) RoundedCornerShape(32.dp) else RoundedCornerShape(
            topStart = 20.dp,
            topEnd = 20.dp
        ),
        color = if (floating) {
            MaterialTheme.colorScheme.surfaceContainerHigh.copy(alpha = 0.95f)
        } else {
            MaterialTheme.colorScheme.surfaceContainer
        },
        tonalElevation = if (floating) 12.dp else 0.dp,
        shadowElevation = if (floating) 16.dp else 0.dp
    ) {
        Row(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = if (floating) 8.dp else 12.dp),
            horizontalArrangement = Arrangement.SpaceAround,
            verticalAlignment = Alignment.CenterVertically
        ) {
            destinations.forEach { dest ->
                val selected = currentRoute == dest.route
                val scale by animateFloatAsState(
                    targetValue = if (selected) {
                        if (floating) 1.07f else 1.04f
                    } else {
                        1f
                    },
                    animationSpec = tween(
                        durationMillis = 220,
                        easing = FastOutSlowInEasing
                    ),
                    label = "bottomBarScale"
                )

                val containerColor by animateColorAsState(
                    targetValue = if (selected) {
                        if (floating) MaterialTheme.colorScheme.primaryContainer
                        else MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.55f)
                    } else {
                        Color.Transparent
                    },
                    animationSpec = tween(
                        durationMillis = 240,
                        easing = FastOutSlowInEasing
                    ),
                    label = "bottomBarContainerColor"
                )

                Box(
                    modifier = Modifier
                        .weight(1f)
                        .height(if (floating) 56.dp else 52.dp)
                        .padding(horizontal = 4.dp)
                        .clip(CircleShape)
                        .background(containerColor)
                        .clickable(enabled = !selected) { onNavigate(dest.route) },
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = if (selected) dest.selectedIcon else dest.icon,
                        contentDescription = stringResource(id = dest.labelRes),
                        modifier = Modifier
                            .size(28.dp)
                            .scale(scale),
                        tint = if (selected) MaterialTheme.colorScheme.onPrimaryContainer else MaterialTheme.colorScheme.onSurfaceVariant.copy(
                            alpha = 0.7f
                        )
                    )
                }
            }
        }
    }
}

private enum class AppDestination(
    val route: String,
    val icon: ImageVector,
    val selectedIcon: ImageVector,
    val labelRes: Int
) {
    Home(AppRoute.Home, Icons.Rounded.Home, Icons.Rounded.Home, CoreR.string.section_home),
    Modules(
        AppRoute.Modules,
        Icons.Rounded.Extension,
        Icons.Rounded.Extension,
        CoreR.string.modules
    ),
    Superuser(
        AppRoute.Superuser,
        Icons.Rounded.Shield,
        Icons.Rounded.Shield,
        CoreR.string.superuser
    ),
    Logs(AppRoute.Logs, Icons.Rounded.Terminal, Icons.Rounded.Terminal, CoreR.string.logs),
    Settings(
        AppRoute.Settings,
        Icons.Rounded.Settings,
        Icons.Rounded.Settings,
        CoreR.string.settings
    );

    companion object {
        val entries = values().toList()
    }
}

data class RouteProcessTopBarState(
    val running: Boolean = false,
    val success: Boolean = false,
    val hasResult: Boolean = false
)

private data class TopBarLabelState(
    val title: String,
    val subtitle: String?
)

private object AppRoute {
    const val Home = "home";
    const val Modules = "modules";
    const val Superuser = "superuser";
    const val History = "history";
    const val Logs = "logs";
    const val Settings = "settings";
    const val Theme = "theme";
    const val DenyList = "denylist";
    const val Install = "install";
    const val FlashPattern = "flash/{action}?uri={uri}";
    const val ModuleActionPattern = "module-action/{id}/{name}"
    fun flash(a: String, u: String?): String =
        "flash/${Uri.encode(a)}?uri=${u?.let { Uri.encode(it) } ?: ""}"

    fun moduleAction(i: String, n: String): String =
        "module-action/${Uri.encode(i)}/${Uri.encode(n)}"
}

private fun routePriority(route: String?): Int = when (route) {
    AppRoute.Home -> 0
    AppRoute.Modules -> 1
    AppRoute.Superuser -> 2
    AppRoute.Logs -> 3
    AppRoute.Settings -> 4
    AppRoute.Theme -> 5
    else -> 6
}

private fun routeDirection(
    initial: String?,
    target: String?,
    rootRouteOrder: Map<String, Int>
): AnimatedContentTransitionScope.SlideDirection {
    val initialRootIndex = rootRouteOrder[initial]
    val targetRootIndex = rootRouteOrder[target]
    if (initialRootIndex != null && targetRootIndex != null) {
        return if (targetRootIndex >= initialRootIndex) {
            AnimatedContentTransitionScope.SlideDirection.Left
        } else {
            AnimatedContentTransitionScope.SlideDirection.Right
        }
    }

    return if (routePriority(target) >= routePriority(initial)) {
        AnimatedContentTransitionScope.SlideDirection.Left
    } else {
        AnimatedContentTransitionScope.SlideDirection.Right
    }
}

private fun rootRouteDistance(
    initial: String?,
    target: String?,
    rootRouteOrder: Map<String, Int>
): Int? {
    val initialRootIndex = rootRouteOrder[initial] ?: return null
    val targetRootIndex = rootRouteOrder[target] ?: return null
    return abs(targetRootIndex - initialRootIndex)
}

private fun AnimatedContentTransitionScope<*>.routeEnterTransition(
    direction: AnimatedContentTransitionScope.SlideDirection,
    rootTabDistance: Int?
): EnterTransition {
    if (rootTabDistance != null) {
        if (rootTabDistance == 0) return EnterTransition.None
        val distance = rootTabDistance.coerceAtLeast(1)
        val offsetDivisor = when (distance) {
            1 -> ROOT_TAB_SLIDE_OFFSET_DIVISOR_NEAR
            2 -> ROOT_TAB_SLIDE_OFFSET_DIVISOR_MID
            else -> ROOT_TAB_SLIDE_OFFSET_DIVISOR_FAR
        }
        val duration = ROOT_ENTER_BASE_DURATION_MS + (distance - 1) * ROOT_ENTER_DISTANCE_STEP_MS
        val distanceOffsetMultiplier = 1f + (distance - 1) * ROOT_TAB_OFFSET_DISTANCE_MULTIPLIER_STEP
        return slideInHorizontally(
            initialOffsetX = {
                val baseOffset = it / offsetDivisor
                val offset = (baseOffset * distanceOffsetMultiplier).toInt().coerceAtLeast(1)
                if (direction == AnimatedContentTransitionScope.SlideDirection.Left) offset else -offset
            },
            animationSpec = tween(
                durationMillis = duration,
                easing = FastOutSlowInEasing
            )
        ) + fadeIn(
            animationSpec = tween(
                durationMillis = ROOT_FADE_IN_DURATION_MS,
                delayMillis = ROOT_FADE_IN_DELAY_MS + (distance - 1) * ROOT_FADE_IN_DISTANCE_STEP_MS,
                easing = LinearOutSlowInEasing
            )
        ) + scaleIn(
            initialScale = ROOT_SCALE_IN_START,
            animationSpec = tween(
                durationMillis = duration,
                easing = FastOutSlowInEasing
            )
        )
    }
    return slideIntoContainer(
        towards = direction,
        animationSpec = tween(
            durationMillis = SCREEN_TRANSITION_DURATION_MS,
            easing = FastOutSlowInEasing
        )
    ) + fadeIn(
        animationSpec = tween(
            durationMillis = 190,
            easing = LinearOutSlowInEasing
        )
    )
}

private fun AnimatedContentTransitionScope<*>.routeExitTransition(
    direction: AnimatedContentTransitionScope.SlideDirection,
    rootTabDistance: Int?
): ExitTransition {
    if (rootTabDistance != null) {
        if (rootTabDistance == 0) return ExitTransition.None
        val distance = rootTabDistance.coerceAtLeast(1)
        val offsetDivisor = when (distance) {
            1 -> ROOT_TAB_SLIDE_OFFSET_DIVISOR_NEAR
            2 -> ROOT_TAB_SLIDE_OFFSET_DIVISOR_MID
            else -> ROOT_TAB_SLIDE_OFFSET_DIVISOR_FAR
        }
        val duration = ROOT_EXIT_BASE_DURATION_MS + (distance - 1) * ROOT_EXIT_DISTANCE_STEP_MS
        val distanceOffsetMultiplier = 1f + (distance - 1) * ROOT_TAB_OFFSET_DISTANCE_MULTIPLIER_STEP
        return slideOutHorizontally(
            targetOffsetX = {
                val baseOffset = it / offsetDivisor
                val offset = (baseOffset * distanceOffsetMultiplier).toInt().coerceAtLeast(1)
                if (direction == AnimatedContentTransitionScope.SlideDirection.Left) -offset else offset
            },
            animationSpec = tween(
                durationMillis = duration,
                easing = FastOutLinearInEasing
            )
        ) + fadeOut(
            animationSpec = tween(
                durationMillis = ROOT_FADE_OUT_DURATION_MS + (distance - 1) * ROOT_FADE_OUT_DISTANCE_STEP_MS,
                easing = FastOutLinearInEasing
            )
        ) + scaleOut(
            targetScale = ROOT_SCALE_OUT_TARGET,
            animationSpec = tween(
                durationMillis = duration,
                easing = FastOutLinearInEasing
            )
        )
    }
    return slideOutOfContainer(
        towards = direction,
        animationSpec = tween(
            durationMillis = SCREEN_TRANSITION_DURATION_MS,
            easing = FastOutSlowInEasing
        )
    ) + fadeOut(
        animationSpec = tween(
            durationMillis = 150,
            easing = FastOutLinearInEasing
        )
    )
}

@Composable
fun fallbackColorScheme(darkTheme: Boolean): ColorScheme {
    val color = @Composable { id: Int -> colorResource(id = id) }
    return if (darkTheme) darkColorScheme(
        primary = color(R.color.theme_default_primary),
        surface = color(R.color.theme_default_surface),
        background = color(R.color.theme_default_surface_surface_variant)
    )
    else lightColorScheme(
        primary = color(R.color.theme_default_primary),
        surface = color(R.color.theme_default_surface),
        background = color(R.color.theme_default_surface_surface_variant)
    )
}

private val LOGS_RESERVED_BOTTOM_SPACE = 120.dp
private const val SCREEN_TRANSITION_DURATION_MS = 280
private const val ROOT_ENTER_BASE_DURATION_MS = 250
private const val ROOT_ENTER_DISTANCE_STEP_MS = 45
private const val ROOT_EXIT_BASE_DURATION_MS = 230
private const val ROOT_EXIT_DISTANCE_STEP_MS = 35
private const val ROOT_TAB_SLIDE_OFFSET_DIVISOR_NEAR = 8
private const val ROOT_TAB_SLIDE_OFFSET_DIVISOR_MID = 6
private const val ROOT_TAB_SLIDE_OFFSET_DIVISOR_FAR = 5
private const val ROOT_TAB_OFFSET_DISTANCE_MULTIPLIER_STEP = 0.12f
private const val ROOT_FADE_IN_DURATION_MS = 210
private const val ROOT_FADE_IN_DELAY_MS = 10
private const val ROOT_FADE_IN_DISTANCE_STEP_MS = 10
private const val ROOT_FADE_OUT_DURATION_MS = 170
private const val ROOT_FADE_OUT_DISTANCE_STEP_MS = 10
private const val ROOT_SCALE_IN_START = 0.992f
private const val ROOT_SCALE_OUT_TARGET = 0.998f
