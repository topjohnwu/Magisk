package com.topjohnwu.magisk.ui.compose

import android.net.Uri
import android.os.Build
import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.ArrowBack
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
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
import androidx.navigation.NavDestination
import androidx.navigation.NavGraph.Companion.findStartDestination
import androidx.navigation.NavType
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import androidx.navigation.navArgument
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.R as CoreR
import com.topjohnwu.magisk.ui.compose.denylist.DenyListScreen
import com.topjohnwu.magisk.ui.compose.flash.FlashScreen
import com.topjohnwu.magisk.ui.compose.home.HomeScreen
import com.topjohnwu.magisk.ui.compose.install.InstallScreen
import com.topjohnwu.magisk.ui.compose.logs.LogsScreen
import com.topjohnwu.magisk.ui.compose.superuser.SuperuserLogsScreen
import com.topjohnwu.magisk.ui.compose.module.ModuleActionScreen
import com.topjohnwu.magisk.ui.compose.module.ModuleScreen
import com.topjohnwu.magisk.ui.compose.settings.SettingsScreen
import com.topjohnwu.magisk.ui.compose.superuser.SuperuserScreen
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MagiskAppContainer(
    useDynamicColor: Boolean = Build.VERSION.SDK_INT >= Build.VERSION_CODES.S,
    darkTheme: Boolean = isSystemInDarkTheme(),
    openSection: String? = null
) {
    val context = LocalContext.current
    val colorScheme = when {
        useDynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
            if (darkTheme) dynamicDarkColorScheme(context) else dynamicLightColorScheme(context)
        }
        else -> fallbackColorScheme(darkTheme)
    }

    MaterialTheme(colorScheme = colorScheme) {
        val navController = rememberNavController()
        val snackbarHostState = remember { SnackbarHostState() }

        val rootDestinations = remember {
            buildList {
                add(AppDestination.Home)
                if (Info.isRooted && Info.env.isActive) add(AppDestination.Modules)
                if (Info.showSuperUser) add(AppDestination.Superuser)
                add(AppDestination.Logs)
                add(AppDestination.Settings)
            }
        }
        val rootRoutes = rootDestinations.map { it.route }.toSet()
        val backStackEntry by navController.currentBackStackEntryAsState()
        val currentDestination = backStackEntry?.destination
        val currentRoute = currentDestination?.route ?: AppRoute.Home
        val currentRoot = rootDestinations.firstOrNull { it.route == currentRoute } ?: AppDestination.Home
        val isRootRoute = currentRoute in rootRoutes

        LaunchedEffect(openSection) {
            val route = when (openSection) {
                Const.Nav.SUPERUSER -> AppRoute.Superuser
                Const.Nav.MODULES -> AppRoute.Modules
                Const.Nav.SETTINGS -> AppRoute.Settings
                else -> null
            } ?: return@LaunchedEffect
            
            navController.navigate(route) {
                popUpTo(navController.graph.findStartDestination().id) { saveState = true }
                launchSingleTop = true
                restoreState = true
            }
        }

        Scaffold(
            modifier = Modifier.fillMaxSize(),
            topBar = {
                // Fissa strutturale: occupa il suo spazio in alto
                MagiskFloatingTopBar(
                    currentRoute = currentRoute,
                    currentRoot = currentRoot,
                    isRootRoute = isRootRoute,
                    backStackEntry = backStackEntry,
                    onBack = { navController.popBackStack() },
                    onOpenSettings = { navController.navigate(AppRoute.Settings) }
                )
            },
            snackbarHost = {
                SnackbarHost(hostState = snackbarHostState) { data -> ExpressiveSnackbar(data) }
            }
        ) { paddingValues ->
            Box(modifier = Modifier
                .fillMaxSize()
                .padding(top = paddingValues.calculateTopPadding())) { // Gestione spazio Top Bar
                
                NavHost(
                    navController = navController, 
                    startDestination = AppRoute.Home, 
                    modifier = Modifier.fillMaxSize(),
                    enterTransition = {
                        if (initialState.destination.route in rootRoutes && targetState.destination.route in rootRoutes) {
                            fadeIn(tween(400)) + scaleIn(initialScale = 0.92f, animationSpec = tween(400, easing = EaseOutQuart))
                        } else {
                            slideIntoContainer(
                                AnimatedContentTransitionScope.SlideDirection.Start, 
                                animationSpec = tween(500, easing = EaseOutQuart)
                            ) + fadeIn(animationSpec = tween(300))
                        }
                    },
                    exitTransition = {
                        if (initialState.destination.route in rootRoutes && targetState.destination.route in rootRoutes) {
                            fadeOut(tween(300)) + scaleOut(targetScale = 0.95f, animationSpec = tween(300))
                        } else {
                            slideOutOfContainer(
                                AnimatedContentTransitionScope.SlideDirection.Start, 
                                animationSpec = tween(500, easing = EaseOutQuart)
                            ) + fadeOut(animationSpec = tween(300))
                        }
                    },
                    popEnterTransition = {
                        if (initialState.destination.route in rootRoutes && targetState.destination.route in rootRoutes) {
                            fadeIn(tween(400)) + scaleIn(initialScale = 0.92f, animationSpec = tween(400, easing = EaseOutQuart))
                        } else {
                            slideIntoContainer(
                                AnimatedContentTransitionScope.SlideDirection.End, 
                                animationSpec = tween(500, easing = EaseOutQuart)
                            ) + fadeIn(animationSpec = tween(300))
                        }
                    },
                    popExitTransition = {
                        if (initialState.destination.route in rootRoutes && targetState.destination.route in rootRoutes) {
                            fadeOut(tween(300)) + scaleOut(targetScale = 0.95f, animationSpec = tween(300))
                        } else {
                            slideOutOfContainer(
                                AnimatedContentTransitionScope.SlideDirection.End, 
                                animationSpec = tween(500, easing = EaseOutQuart)
                            ) + fadeOut(animationSpec = tween(300))
                        }
                    }
                ) {
                    composable(AppRoute.Home) { HomeScreen(onOpenInstall = { navController.navigate(AppRoute.Install) }, onOpenUninstall = { navController.navigate(AppRoute.flash(Const.Value.UNINSTALL, null)) }) }
                    composable(AppRoute.Modules) { ModuleScreen(onInstallZip = { uri -> navController.navigate(AppRoute.flash(Const.Value.FLASH_ZIP, uri.toString())) }, onRunAction = { id, name -> navController.navigate(AppRoute.moduleAction(id, name)) }) }
                    composable(AppRoute.Superuser) { SuperuserScreen(onOpenLogs = { navController.navigate(AppRoute.History) }) }
                    composable(AppRoute.Logs) { LogsScreen() }
                    
                    composable(AppRoute.Settings) { SettingsScreen(onOpenDenyList = { navController.navigate(AppRoute.DenyList) }) }
                    composable(AppRoute.History) { SuperuserLogsScreen() }
                    composable(AppRoute.DenyList) { DenyListScreen(onBack = { navController.popBackStack() }) }
                    composable(AppRoute.Install) { InstallScreen(onStartFlash = { action, uri -> navController.navigate(AppRoute.flash(action, uri?.toString())) }) }
                    composable(route = AppRoute.FlashPattern, arguments = listOf(navArgument("action") { type = NavType.StringType }, navArgument("uri") { type = NavType.StringType; nullable = true; defaultValue = null })) { entry ->
                        val action = entry.arguments?.getString("action").orEmpty()
                        val uriArg = entry.arguments?.getString("uri")
                        FlashScreen(action = action, uriArg = uriArg, onBack = { navController.popBackStack() })
                    }
                    composable(route = AppRoute.ModuleActionPattern, arguments = listOf(navArgument("id") { type = NavType.StringType }, navArgument("name") { type = NavType.StringType; defaultValue = "" })) { entry ->
                        val id = entry.arguments?.getString("id").orEmpty()
                        val name = entry.arguments?.getString("name").orEmpty()
                        val safeName = runCatching { Uri.decode(name) }.getOrDefault(name)
                        ModuleActionScreen(actionId = id, actionName = safeName, onBack = { navController.popBackStack() })
                    }
                }


                if (isRootRoute) {
                    Box(modifier = Modifier.align(Alignment.BottomCenter)) {
                        MagiskFloatingBottomBar(destinations = rootDestinations, current = currentDestination, onNavigate = { route ->
                            navController.navigate(route) {
                                popUpTo(navController.graph.findStartDestination().id) { saveState = true }
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
    backStackEntry: androidx.navigation.NavBackStackEntry?,
    onBack: () -> Unit,
    onOpenSettings: () -> Unit
) {
    Surface(
        modifier = Modifier
            .padding(horizontal = 16.dp, vertical = 8.dp)
            .statusBarsPadding()
            .fillMaxWidth()
            .height(64.dp),
        shape = RoundedCornerShape(24.dp),
        color = MaterialTheme.colorScheme.surfaceContainerHighest.copy(alpha = 0.88f),
        tonalElevation = 8.dp,
        shadowElevation = 4.dp
    ) {
        Row(
            modifier = Modifier.fillMaxSize().padding(horizontal = 8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            if (!isRootRoute) {
                IconButton(onClick = onBack) {
                    Icon(Icons.AutoMirrored.Rounded.ArrowBack, contentDescription = null)
                }
            } else {
                Spacer(Modifier.width(12.dp))
            }

            Row(
                modifier = Modifier.weight(1f),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                val displayIcon = when (currentRoute) {
                    AppRoute.DenyList -> Icons.Rounded.Block
                    AppRoute.Install -> Icons.Rounded.Download
                    AppRoute.FlashPattern -> Icons.Rounded.Terminal
                    AppRoute.ModuleActionPattern -> Icons.Rounded.PlayCircle
                    AppRoute.History -> Icons.Rounded.HistoryEdu
                    AppRoute.Settings -> Icons.Rounded.Settings
                    else -> currentRoot.selectedIcon
                }

                Surface(
                    color = MaterialTheme.colorScheme.primary.copy(alpha = 0.1f),
                    shape = RoundedCornerShape(12.dp),
                    modifier = Modifier.size(32.dp)
                ) {
                    Icon(imageVector = displayIcon, contentDescription = null, modifier = Modifier.padding(6.dp), tint = MaterialTheme.colorScheme.primary)
                }

                Text(
                    text = when (currentRoute) {
                        AppRoute.DenyList -> stringResource(id = CoreR.string.denylist)
                        AppRoute.Install -> stringResource(id = CoreR.string.install)
                        AppRoute.FlashPattern -> stringResource(id = CoreR.string.flash_screen_title)
                        AppRoute.ModuleActionPattern -> backStackEntry?.arguments?.getString("name")?.let {
                            runCatching { Uri.decode(it) }.getOrDefault(it)
                        } ?: stringResource(id = CoreR.string.module_action)
                        AppRoute.History -> stringResource(id = CoreR.string.superuser_logs)
                        AppRoute.Settings -> stringResource(id = CoreR.string.settings)
                        else -> stringResource(id = currentRoot.labelRes)
                    },
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Black,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
            }

            if (currentRoute == AppRoute.Home) {
                IconButton(onClick = onOpenSettings) {
                    Icon(Icons.Rounded.Settings, contentDescription = null)
                }
            } else {
                Spacer(Modifier.width(12.dp))
            }
        }
    }
}

@Composable
private fun ExpressiveSnackbar(snackbarData: SnackbarData) {
    Surface(
        modifier = Modifier.padding(16.dp).padding(bottom = 110.dp),
        shape = RoundedCornerShape(24.dp),
        color = MaterialTheme.colorScheme.surfaceContainerHighest,
        tonalElevation = 6.dp,
        shadowElevation = 4.dp
    ) {
        Row(modifier = Modifier.padding(horizontal = 20.dp, vertical = 14.dp), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            Icon(Icons.Rounded.Info, null, tint = MaterialTheme.colorScheme.primary, modifier = Modifier.size(24.dp))
            Text(text = snackbarData.visuals.message, style = MaterialTheme.typography.bodyMedium, fontWeight = FontWeight.Medium, color = MaterialTheme.colorScheme.onSurface, modifier = Modifier.weight(1f))
            if (snackbarData.visuals.actionLabel != null) {
                TextButton(onClick = { snackbarData.performAction() }) { Text(snackbarData.visuals.actionLabel!!, fontWeight = FontWeight.Bold) }
            }
        }
    }
}

@Composable
private fun MagiskFloatingBottomBar(destinations: List<AppDestination>, current: NavDestination?, onNavigate: (String) -> Unit) {
    Surface(
        modifier = Modifier
            .padding(horizontal = 24.dp, vertical = 28.dp)
            .navigationBarsPadding(),
        shape = RoundedCornerShape(28.dp),
        color = MaterialTheme.colorScheme.surfaceContainerHighest.copy(alpha = 0.88f),
        tonalElevation = 8.dp,
        shadowElevation = 12.dp
    ) {
        Row(
            modifier = Modifier.fillMaxWidth().height(80.dp).padding(horizontal = 12.dp),
            horizontalArrangement = Arrangement.SpaceAround,
            verticalAlignment = Alignment.CenterVertically
        ) {
            destinations.forEach { dest ->
                val selected = current?.route == dest.route
                val scale by animateFloatAsState(targetValue = if (selected) { 1.2f } else { 1f }, animationSpec = spring(dampingRatio = Spring.DampingRatioMediumBouncy, stiffness = Spring.StiffnessLow), label = "iconScale")
                Box(modifier = Modifier.height(64.dp).weight(1f).clip(CircleShape).clickable { onNavigate(dest.route) }, contentAlignment = Alignment.Center) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.Center) {
                        Icon(imageVector = if (selected) dest.selectedIcon else dest.icon, contentDescription = stringResource(id = dest.labelRes), modifier = Modifier.size(26.dp).scale(scale), tint = if (selected) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.onSurfaceVariant)
                    }
                }
            }
        }
    }
}

private enum class AppDestination(val route: String, val icon: ImageVector, val selectedIcon: ImageVector, val labelRes: Int) {
    Home(AppRoute.Home, Icons.Rounded.Home, Icons.Rounded.Home, CoreR.string.section_home),
    Modules(AppRoute.Modules, Icons.Rounded.Extension, Icons.Rounded.Extension, CoreR.string.modules),
    Superuser(AppRoute.Superuser, Icons.Rounded.Shield, Icons.Rounded.Shield, CoreR.string.superuser),
    Logs(AppRoute.Logs, Icons.Rounded.Terminal, Icons.Rounded.Terminal, CoreR.string.logs),
    Settings(AppRoute.Settings, Icons.Rounded.Settings, Icons.Rounded.Settings, CoreR.string.settings);
    companion object { val entries = values().toList() }
}

private object AppRoute {
    const val Home = "home"; const val Modules = "modules"; const val Superuser = "superuser"; const val History = "history"; const val Logs = "logs"; const val Settings = "settings"; const val DenyList = "denylist"; const val Install = "install"; const val FlashPattern = "flash/{action}?uri={uri}"; const val ModuleActionPattern = "module-action/{id}/{name}"
    fun flash(a: String, u: String?): String = "flash/${Uri.encode(a)}?uri=${u?.let { Uri.encode(it) } ?: ""}"
    fun moduleAction(i: String, n: String): String = "module-action/${Uri.encode(i)}/${Uri.encode(n)}"
}

@Composable
fun fallbackColorScheme(darkTheme: Boolean): ColorScheme {
    val color = @Composable { id: Int -> colorResource(id = id) }
    return if (darkTheme) darkColorScheme(primary = color(R.color.theme_default_primary), surface = color(R.color.theme_default_surface), background = color(R.color.theme_default_surface_surface_variant))
    else lightColorScheme(primary = color(R.color.theme_default_primary), surface = color(R.color.theme_default_surface), background = color(R.color.theme_default_surface_surface_variant))
}
