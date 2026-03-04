package com.topjohnwu.magisk.ui.settings

import androidx.annotation.RawRes
import androidx.annotation.StringRes
import androidx.appcompat.app.AppCompatDelegate
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.GridItemSpan
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.BrightnessAuto
import androidx.compose.material.icons.rounded.Brush
import androidx.compose.material.icons.rounded.Check
import androidx.compose.material.icons.rounded.CheckCircle
import androidx.compose.material.icons.rounded.ChevronRight
import androidx.compose.material.icons.rounded.DarkMode
import androidx.compose.material.icons.rounded.LightMode
import androidx.compose.material.icons.rounded.Palette
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.key
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.graphics.luminance
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import coil.compose.AsyncImage
import coil.decode.SvgDecoder
import coil.request.ImageRequest
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.ui.theme.Theme
import com.topjohnwu.magisk.core.R as CoreR

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ThemeScreen(
    onThemeChanged: () -> Unit = {}
) {
    var currentTheme by remember { mutableStateOf(Theme.selected) }
    var currentDarkMode by remember { mutableIntStateOf(Config.darkTheme) }
    var customColors by remember { mutableStateOf(CustomThemeColors.fromConfig()) }
    var customDraftColors by remember { mutableStateOf(customColors) }
    var showCustomEditorSheet by remember { mutableStateOf(false) }
    val themes = remember { Theme.displayOrder }

    Box(modifier = Modifier.fillMaxSize()) {
        LazyVerticalGrid(
            columns = GridCells.Fixed(2),
            contentPadding = PaddingValues(start = 20.dp, end = 20.dp, top = 12.dp, bottom = 140.dp),
            horizontalArrangement = Arrangement.spacedBy(16.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp),
            modifier = Modifier.fillMaxSize()
        ) {
            item(span = { GridItemSpan(2) }) {
                ThemeHeader()
            }
            item(span = { GridItemSpan(2) }) {
                DarkModeSection(
                    currentDarkMode = currentDarkMode,
                    onDarkModeSelected = { mode ->
                        if (currentDarkMode == mode) return@DarkModeSection
                        val previousMode = currentDarkMode
                        currentDarkMode = mode
                        Config.darkTheme = mode
                        AppCompatDelegate.setDefaultNightMode(
                            if (mode == Config.Value.DARK_THEME_AMOLED) {
                                AppCompatDelegate.MODE_NIGHT_YES
                            } else {
                                mode
                            }
                        )
                        if (previousMode == Config.Value.DARK_THEME_AMOLED ||
                            mode == Config.Value.DARK_THEME_AMOLED
                        ) {
                            onThemeChanged()
                        }
                    }
                )
            }

            items(themes) { theme ->
                ThemeCard(
                    theme = theme,
                    isSelected = theme == currentTheme,
                    customColors = if (theme == Theme.Custom) customColors else null,
                    onClick = {
                        if (theme == currentTheme) {
                            if (theme == Theme.Custom) {
                                customDraftColors = customColors
                                showCustomEditorSheet = true
                            }
                            return@ThemeCard
                        }
                        if (theme == Theme.Custom) {
                            customDraftColors = customColors
                            showCustomEditorSheet = true
                            return@ThemeCard
                        }
                        currentTheme = theme
                        Config.themeOrdinal = if (theme == Theme.Default) -1 else theme.ordinal
                        onThemeChanged()
                    }
                )
            }
        }

        if (showCustomEditorSheet) {
            ModalBottomSheet(
                onDismissRequest = {
                    customDraftColors = customColors
                    showCustomEditorSheet = false
                },
                shape = RoundedCornerShape(topStart = 28.dp, topEnd = 28.dp),
                containerColor = MaterialTheme.colorScheme.surfaceContainer,
                dragHandle = { BottomSheetDefaults.DragHandle() }
            ) {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .navigationBarsPadding()
                        .imePadding()
                        .verticalScroll(rememberScrollState())
                ) {
                    CustomThemeEditor(
                        colors = customDraftColors,
                        onColorChanged = { slot, colorInt ->
                            customDraftColors = customDraftColors.update(slot, colorInt)
                        }
                    )
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(horizontal = 16.dp, vertical = 10.dp),
                        horizontalArrangement = Arrangement.spacedBy(10.dp)
                    ) {
                        OutlinedButton(
                            onClick = {
                                customDraftColors = customColors
                                showCustomEditorSheet = false
                            },
                            modifier = Modifier
                                .weight(1f)
                                .height(52.dp),
                            shape = RoundedCornerShape(16.dp)
                        ) {
                            Text(stringResource(id = android.R.string.cancel), fontWeight = FontWeight.Bold)
                        }
                        Button(
                            onClick = {
                                customDraftColors.persistToConfig()
                                customColors = customDraftColors
                                currentTheme = Theme.Custom
                                Config.themeOrdinal = Theme.Custom.ordinal
                                onThemeChanged()
                                showCustomEditorSheet = false
                            },
                            modifier = Modifier
                                .weight(1f)
                                .height(52.dp),
                            shape = RoundedCornerShape(16.dp)
                        ) {
                            Text(stringResource(id = CoreR.string.apply), fontWeight = FontWeight.Black)
                        }
                    }
                    Spacer(Modifier.height(8.dp))
                }
            }
        }
    }
}

@Composable
private fun DarkModeSection(
    currentDarkMode: Int,
    onDarkModeSelected: (Int) -> Unit
) {
    var showModeMenu by remember { mutableStateOf(false) }
    val options = listOf(
        DarkModeMenuOption(
            mode = AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM,
            icon = Icons.Rounded.BrightnessAuto,
            label = stringResource(id = CoreR.string.settings_dark_mode_system),
            subtitle = stringResource(id = CoreR.string.theme_dark_mode_subtitle_system)
        ),
        DarkModeMenuOption(
            mode = AppCompatDelegate.MODE_NIGHT_NO,
            icon = Icons.Rounded.LightMode,
            label = stringResource(id = CoreR.string.settings_dark_mode_light),
            subtitle = stringResource(id = CoreR.string.theme_dark_mode_subtitle_light)
        ),
        DarkModeMenuOption(
            mode = AppCompatDelegate.MODE_NIGHT_YES,
            icon = Icons.Rounded.DarkMode,
            label = stringResource(id = CoreR.string.settings_dark_mode_dark),
            subtitle = stringResource(id = CoreR.string.theme_dark_mode_subtitle_dark)
        ),
        DarkModeMenuOption(
            mode = Config.Value.DARK_THEME_AMOLED,
            icon = Icons.Rounded.DarkMode,
            label = stringResource(id = CoreR.string.settings_dark_mode_amoled),
            subtitle = stringResource(id = CoreR.string.theme_dark_mode_subtitle_amoled)
        )
    )
    val selected = options.firstOrNull { it.mode == currentDarkMode } ?: options.first()

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(bottom = 8.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            text = stringResource(id = CoreR.string.settings_dark_mode_title).uppercase(),
            style = MaterialTheme.typography.labelLarge,
            fontWeight = FontWeight.Black,
            letterSpacing = 1.2.sp,
            color = MaterialTheme.colorScheme.outline,
            modifier = Modifier.padding(horizontal = 8.dp)
        )
        ElevatedCard(
            modifier = Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(20.dp))
                .clickable { showModeMenu = true },
            shape = RoundedCornerShape(20.dp),
            colors = CardDefaults.elevatedCardColors(
                containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
            ),
            elevation = CardDefaults.elevatedCardElevation(defaultElevation = 2.dp)
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 14.dp, vertical = 12.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Surface(
                    shape = RoundedCornerShape(12.dp),
                    color = MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.75f),
                    modifier = Modifier.size(34.dp)
                ) {
                    Icon(
                        imageVector = selected.icon,
                        contentDescription = null,
                        modifier = Modifier.padding(7.dp),
                        tint = MaterialTheme.colorScheme.onPrimaryContainer
                    )
                }
                Spacer(Modifier.width(12.dp))
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = selected.label,
                        style = MaterialTheme.typography.labelLarge,
                        fontWeight = FontWeight.Black
                    )
                    Text(
                        text = selected.subtitle,
                        style = MaterialTheme.typography.labelMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                Surface(
                    shape = RoundedCornerShape(8.dp),
                    color = MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.6f)
                ) {
                    Text(
                        text = stringResource(id = CoreR.string.theme_mode).uppercase(),
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp),
                        style = MaterialTheme.typography.labelSmall,
                        fontWeight = FontWeight.Black,
                        color = MaterialTheme.colorScheme.primary
                    )
                }
                Spacer(Modifier.width(6.dp))
                Icon(
                    imageVector = Icons.Rounded.ChevronRight,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }

        if (showModeMenu) {
            AlertDialog(
                onDismissRequest = { showModeMenu = false },
                title = {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Surface(
                            shape = RoundedCornerShape(10.dp),
                            color = MaterialTheme.colorScheme.primaryContainer,
                            modifier = Modifier.size(28.dp)
                        ) {
                            Icon(
                                imageVector = selected.icon,
                                contentDescription = null,
                                modifier = Modifier.padding(6.dp),
                                tint = MaterialTheme.colorScheme.onPrimaryContainer
                            )
                        }
                        Spacer(Modifier.width(10.dp))
                        Text(
                            stringResource(id = CoreR.string.settings_dark_mode_title),
                            fontWeight = FontWeight.Black
                        )
                    }
                },
                text = {
                    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                        options.forEach { option ->
                            Surface(
                                modifier = Modifier.fillMaxWidth(),
                                shape = RoundedCornerShape(16.dp),
                                color = if (option.mode == currentDarkMode) {
                                    MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.42f)
                                } else {
                                    MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.24f)
                                },
                                onClick = {
                                    showModeMenu = false
                                    onDarkModeSelected(option.mode)
                                }
                            ) {
                                Row(
                                    modifier = Modifier.padding(horizontal = 12.dp, vertical = 10.dp),
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    Surface(
                                        shape = RoundedCornerShape(10.dp),
                                        color = if (option.mode == currentDarkMode) {
                                            MaterialTheme.colorScheme.primary
                                        } else {
                                            MaterialTheme.colorScheme.surfaceContainerHighest
                                        },
                                        modifier = Modifier.size(30.dp)
                                    ) {
                                        Icon(
                                            imageVector = option.icon,
                                            contentDescription = null,
                                            modifier = Modifier.padding(6.dp),
                                            tint = if (option.mode == currentDarkMode) {
                                                MaterialTheme.colorScheme.onPrimary
                                            } else {
                                                MaterialTheme.colorScheme.onSurfaceVariant
                                            }
                                        )
                                    }
                                    Spacer(Modifier.width(10.dp))
                                    Column(modifier = Modifier.weight(1f)) {
                                        Text(
                                            text = option.label,
                                            style = MaterialTheme.typography.bodyLarge,
                                            fontWeight = if (option.mode == currentDarkMode) {
                                                FontWeight.Black
                                            } else {
                                                FontWeight.Medium
                                            }
                                        )
                                        Text(
                                            text = option.subtitle,
                                            style = MaterialTheme.typography.labelMedium,
                                            color = MaterialTheme.colorScheme.onSurfaceVariant
                                        )
                                    }
                                    if (option.mode == currentDarkMode) {
                                        Surface(
                                            shape = CircleShape,
                                            color = MaterialTheme.colorScheme.primary,
                                            modifier = Modifier.size(22.dp)
                                        ) {
                                            Icon(
                                                imageVector = Icons.Rounded.Check,
                                                contentDescription = null,
                                                modifier = Modifier.padding(4.dp),
                                                tint = MaterialTheme.colorScheme.onPrimary
                                            )
                                        }
                                    }
                                }
                            }
                        }
                    }
                },
                confirmButton = {},
                shape = RoundedCornerShape(24.dp),
                containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
            )
        }
    }
}

private data class DarkModeMenuOption(
    val mode: Int,
    val icon: ImageVector,
    val label: String,
    val subtitle: String
)

@Composable
private fun ThemeHeader() {
    Column(modifier = Modifier.padding(bottom = 8.dp)) {
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.padding(horizontal = 8.dp, vertical = 12.dp)
        ) {
            Surface(
                color = MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.7f),
                shape = RoundedCornerShape(12.dp),
                modifier = Modifier.size(32.dp)
            ) {
                Icon(
                    Icons.Rounded.Palette, null,
                    modifier = Modifier.padding(6.dp),
                    tint = MaterialTheme.colorScheme.onPrimaryContainer
                )
            }
            Spacer(Modifier.width(16.dp))
            Text(
                text = stringResource(id = CoreR.string.theme_appearance).uppercase(),
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.Black,
                letterSpacing = 1.2.sp,
                color = MaterialTheme.colorScheme.outline
            )
        }
        
        ElevatedCard(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(28.dp),
            colors = CardDefaults.elevatedCardColors(
                containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
            )
        ) {
            Row(
                modifier = Modifier.padding(24.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Icon(
                    Icons.Rounded.Brush, null,
                    modifier = Modifier.size(40.dp),
                    tint = MaterialTheme.colorScheme.primary
                )
                Spacer(Modifier.width(20.dp))
                Column {
                    Text(
                        stringResource(id = CoreR.string.theme_custom_themes),
                        style = MaterialTheme.typography.titleLarge,
                        fontWeight = FontWeight.Black
                    )
                    Text(
                        stringResource(id = CoreR.string.theme_personalize_experience),
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }
}

@Composable
private fun ThemeCard(
    theme: Theme,
    isSelected: Boolean,
    customColors: CustomThemeColors? = null,
    onClick: () -> Unit
) {
    val previewGradient = when (theme) {
        Theme.Piplup -> listOf(Color(0xFF0061A4), Color(0xFF7AC8FF))
        Theme.PiplupAmoled -> listOf(Color(0xFF050505), Color(0xFF0B6AAE))
        Theme.Rayquaza -> listOf(Color(0xFF006D3B), Color(0xFF38B46C))
        Theme.Zapdos -> listOf(Color(0xFF7A6500), Color(0xFFFFD447))
        Theme.Charmeleon -> listOf(Color(0xFFBF0000), Color(0xFFFF8A5A))
        Theme.Mew -> listOf(Color(0xFF924271), Color(0xFFD07AB4))
        Theme.Salamence -> listOf(Color(0xFF006782), Color(0xFF4CA8C7))
        Theme.Fraxure -> listOf(Color(0xFF3D5467), Color(0xFF93A9BD))
        Theme.Default -> listOf(
            Color(0xFFE85CF0),
            Color(0xFF8A7BFF),
            Color(0xFF58B8FF),
            Color(0xFF57E2E7),
            Color(0xFF62EB9A),
            Color(0xFFB6EF64)
        )
        Theme.Custom -> listOf(
            Color(customColors?.lightPrimary ?: Config.themeCustomLightPrimary),
            Color(customColors?.darkPrimary ?: Config.themeCustomDarkPrimary)
        )
    }
    val accentColor = previewGradient.first()
    val watermarkAlpha = if (theme == Theme.Default) 0.34f else 0.24f
    val iconContainerColor = when (theme) {
        Theme.Piplup -> Color(0xFFD1E4FF)
        Theme.PiplupAmoled -> Color(0xFF0D2B47)
        Theme.Rayquaza -> Color(0xFF96F7B4)
        Theme.Zapdos -> Color(0xFFFFE264)
        Theme.Charmeleon -> Color(0xFFFFDAD4)
        Theme.Mew -> Color(0xFFFFD8E9)
        Theme.Salamence -> Color(0xFFBBE9FF)
        Theme.Fraxure -> Color(0xFFD8E7F3)
        Theme.Default -> Color(0xFFD9E2EA)
        Theme.Custom -> Color(customColors?.lightSecondary ?: Config.themeCustomLightSecondary)
    }
    val iconTint = if (iconContainerColor.luminance() > 0.45f) Color(0xFF111827) else Color.White
    val containerColor = if (isSelected) {
        MaterialTheme.colorScheme.surfaceContainerHighest
    } else {
        MaterialTheme.colorScheme.surfaceContainerHigh
    }

    ElevatedCard(
        modifier = Modifier
            .fillMaxWidth()
            .height(160.dp)
            .clip(RoundedCornerShape(24.dp))
            .clickable { onClick() },
        shape = RoundedCornerShape(24.dp),
        colors = CardDefaults.elevatedCardColors(containerColor = containerColor),
        elevation = CardDefaults.elevatedCardElevation(defaultElevation = if (isSelected) 8.dp else 2.dp)
    ) {
        Box(modifier = Modifier.fillMaxSize()) {
            Column(modifier = Modifier.fillMaxSize()) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f)
                        .background(Brush.verticalGradient(previewGradient)),
                    contentAlignment = Alignment.Center
                ) {
                    ThemeWatermark(
                        theme = theme,
                        modifier = Modifier
                            .fillMaxSize()
                            .padding(horizontal = 12.dp, vertical = 8.dp)
                            .alpha(watermarkAlpha)
                    )
                    Surface(
                        color = iconContainerColor,
                        shape = RoundedCornerShape(10.dp),
                        modifier = Modifier
                            .align(Alignment.BottomEnd)
                            .padding(end = 6.dp, bottom = 8.dp)
                            .size(24.dp)
                    ) {
                        Icon(
                            painter = painterResource(id = CoreR.drawable.ic_magisk),
                            contentDescription = null,
                            modifier = Modifier.padding(5.dp),
                            tint = iconTint
                        )
                    }
                }

                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(1.dp)
                        .background(MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.35f))
                )
                
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(12.dp),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        text = theme.themeName,
                        style = MaterialTheme.typography.labelLarge,
                        fontWeight = FontWeight.Black,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis,
                        modifier = Modifier.weight(1f)
                    )
                    Surface(
                        shape = CircleShape,
                        color = accentColor,
                        modifier = Modifier.size(10.dp)
                    ) {}
                    Spacer(Modifier.width(8.dp))
                    if (isSelected) {
                        Icon(
                            Icons.Rounded.CheckCircle, null,
                            modifier = Modifier.size(20.dp),
                            tint = accentColor
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun CustomThemeEditor(
    colors: CustomThemeColors,
    onColorChanged: (CustomColorSlot, Int) -> Unit
) {
    Column(
        modifier = Modifier.fillMaxWidth().padding(top = 4.dp, bottom = 8.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            text = stringResource(id = CoreR.string.theme_custom_palette).uppercase(),
            style = MaterialTheme.typography.labelLarge,
            fontWeight = FontWeight.Black,
            letterSpacing = 1.1.sp,
            color = MaterialTheme.colorScheme.outline,
            modifier = Modifier.padding(horizontal = 8.dp)
        )
        ElevatedCard(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(24.dp),
            colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHigh)
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                Text(
                    text = stringResource(id = CoreR.string.theme_custom_palette_subtitle),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                CustomColorSlot.entries.forEach { slot ->
                    key(slot) {
                        CustomColorRow(
                            label = stringResource(id = slot.labelRes),
                            colorInt = colors.value(slot),
                            onColorChange = { onColorChanged(slot, it) }
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun CustomColorRow(
    label: String,
    colorInt: Int,
    onColorChange: (Int) -> Unit
) {
    var showEditor by remember { mutableStateOf(false) }
    Surface(
        onClick = { showEditor = true },
        shape = RoundedCornerShape(14.dp),
        color = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.35f),
        modifier = Modifier.fillMaxWidth()
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 12.dp, vertical = 10.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Surface(shape = CircleShape, color = Color(colorInt), modifier = Modifier.size(18.dp)) {}
            Spacer(Modifier.width(10.dp))
            Column(modifier = Modifier.weight(1f)) {
                Text(label, style = MaterialTheme.typography.labelLarge, fontWeight = FontWeight.Bold)
                Text(colorInt.toColorHex(), style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
            }
            TextButton(
                onClick = { showEditor = true },
                contentPadding = PaddingValues(horizontal = 10.dp, vertical = 2.dp)
            ) {
                Text(
                    text = stringResource(id = CoreR.string.edit).uppercase(),
                    style = MaterialTheme.typography.labelSmall,
                    fontWeight = FontWeight.Black,
                    color = MaterialTheme.colorScheme.primary
                )
            }
        }
    }
    if (showEditor) {
        ColorHexDialog(
            title = label,
            initialColor = colorInt,
            onDismiss = { showEditor = false },
            onConfirm = { color ->
                onColorChange(color)
                showEditor = false
            }
        )
    }
}

@Composable
private fun ColorHexDialog(
    title: String,
    initialColor: Int,
    onDismiss: () -> Unit,
    onConfirm: (Int) -> Unit
) {
    var alpha by remember(initialColor) { mutableIntStateOf(android.graphics.Color.alpha(initialColor)) }
    var red by remember(initialColor) { mutableIntStateOf(android.graphics.Color.red(initialColor)) }
    var green by remember(initialColor) { mutableIntStateOf(android.graphics.Color.green(initialColor)) }
    var blue by remember(initialColor) { mutableIntStateOf(android.graphics.Color.blue(initialColor)) }
    val currentColorInt = remember(alpha, red, green, blue) {
        android.graphics.Color.argb(alpha, red, green, blue)
    }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title, fontWeight = FontWeight.Black) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                    Surface(
                        shape = CircleShape,
                        color = Color(currentColorInt),
                        modifier = Modifier.size(24.dp)
                    ) {}
                    Text(
                        text = currentColorInt.toColorHex(),
                        style = MaterialTheme.typography.labelMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                ColorChannelSlider(
                    label = stringResource(id = CoreR.string.color_channel_alpha),
                    value = alpha,
                    activeColor = MaterialTheme.colorScheme.onSurfaceVariant,
                    onValueChange = { alpha = it }
                )
                ColorChannelSlider(label = stringResource(id = CoreR.string.color_channel_red), value = red, activeColor = Color(0xFFEF5350), onValueChange = { red = it })
                ColorChannelSlider(label = stringResource(id = CoreR.string.color_channel_green), value = green, activeColor = Color(0xFF66BB6A), onValueChange = { green = it })
                ColorChannelSlider(label = stringResource(id = CoreR.string.color_channel_blue), value = blue, activeColor = Color(0xFF42A5F5), onValueChange = { blue = it })
            }
        },
        confirmButton = {
            TextButton(
                onClick = { onConfirm(currentColorInt) }
            ) {
                Text(stringResource(id = CoreR.string.apply), fontWeight = FontWeight.Bold)
            }
        },
        dismissButton = { TextButton(onClick = onDismiss) { Text(stringResource(id = android.R.string.cancel)) } },
        shape = RoundedCornerShape(24.dp),
        containerColor = MaterialTheme.colorScheme.surfaceContainerHigh
    )
}

@Composable
private fun ColorChannelSlider(
    label: String,
    value: Int,
    activeColor: Color,
    onValueChange: (Int) -> Unit
) {
    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = label,
                style = MaterialTheme.typography.labelMedium,
                fontWeight = FontWeight.Bold
            )
            Text(
                text = value.toString(),
                style = MaterialTheme.typography.labelMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        Slider(
            value = value.toFloat(),
            onValueChange = { onValueChange(it.toInt().coerceIn(0, 255)) },
            valueRange = 0f..255f,
            steps = 254,
            colors = SliderDefaults.colors(
                thumbColor = activeColor,
                activeTrackColor = activeColor
            )
        )
    }
}

private enum class CustomColorSlot(@StringRes val labelRes: Int) {
    LightPrimary(CoreR.string.theme_color_light_primary),
    DarkPrimary(CoreR.string.theme_color_dark_primary),
    LightSecondary(CoreR.string.theme_color_light_secondary),
    DarkSecondary(CoreR.string.theme_color_dark_secondary),
    LightSurface(CoreR.string.theme_color_light_surface),
    DarkSurface(CoreR.string.theme_color_dark_surface),
    LightOnSurface(CoreR.string.theme_color_light_on_surface),
    DarkOnSurface(CoreR.string.theme_color_dark_on_surface),
    LightError(CoreR.string.theme_color_light_error),
    DarkError(CoreR.string.theme_color_dark_error)
}

private data class CustomThemeColors(
    val lightPrimary: Int,
    val darkPrimary: Int,
    val lightSecondary: Int,
    val darkSecondary: Int,
    val lightSurface: Int,
    val darkSurface: Int,
    val lightOnSurface: Int,
    val darkOnSurface: Int,
    val lightError: Int,
    val darkError: Int
) {
    fun value(slot: CustomColorSlot): Int = when (slot) {
        CustomColorSlot.LightPrimary -> lightPrimary
        CustomColorSlot.DarkPrimary -> darkPrimary
        CustomColorSlot.LightSecondary -> lightSecondary
        CustomColorSlot.DarkSecondary -> darkSecondary
        CustomColorSlot.LightSurface -> lightSurface
        CustomColorSlot.DarkSurface -> darkSurface
        CustomColorSlot.LightOnSurface -> lightOnSurface
        CustomColorSlot.DarkOnSurface -> darkOnSurface
        CustomColorSlot.LightError -> lightError
        CustomColorSlot.DarkError -> darkError
    }

    fun update(slot: CustomColorSlot, colorInt: Int): CustomThemeColors = when (slot) {
        CustomColorSlot.LightPrimary -> copy(lightPrimary = colorInt)
        CustomColorSlot.DarkPrimary -> copy(darkPrimary = colorInt)
        CustomColorSlot.LightSecondary -> copy(lightSecondary = colorInt)
        CustomColorSlot.DarkSecondary -> copy(darkSecondary = colorInt)
        CustomColorSlot.LightSurface -> copy(lightSurface = colorInt)
        CustomColorSlot.DarkSurface -> copy(darkSurface = colorInt)
        CustomColorSlot.LightOnSurface -> copy(lightOnSurface = colorInt)
        CustomColorSlot.DarkOnSurface -> copy(darkOnSurface = colorInt)
        CustomColorSlot.LightError -> copy(lightError = colorInt)
        CustomColorSlot.DarkError -> copy(darkError = colorInt)
    }

    fun persistToConfig() {
        Config.themeCustomLightPrimary = lightPrimary
        Config.themeCustomDarkPrimary = darkPrimary
        Config.themeCustomLightSecondary = lightSecondary
        Config.themeCustomDarkSecondary = darkSecondary
        Config.themeCustomLightSurface = lightSurface
        Config.themeCustomDarkSurface = darkSurface
        Config.themeCustomLightOnSurface = lightOnSurface
        Config.themeCustomDarkOnSurface = darkOnSurface
        Config.themeCustomLightError = lightError
        Config.themeCustomDarkError = darkError
    }

    companion object {
        fun fromConfig(): CustomThemeColors = CustomThemeColors(
            lightPrimary = Config.themeCustomLightPrimary,
            darkPrimary = Config.themeCustomDarkPrimary,
            lightSecondary = Config.themeCustomLightSecondary,
            darkSecondary = Config.themeCustomDarkSecondary,
            lightSurface = Config.themeCustomLightSurface,
            darkSurface = Config.themeCustomDarkSurface,
            lightOnSurface = Config.themeCustomLightOnSurface,
            darkOnSurface = Config.themeCustomDarkOnSurface,
            lightError = Config.themeCustomLightError,
            darkError = Config.themeCustomDarkError
        )
    }
}

private fun Int.toColorHex(): String = String.format("#%08X", this)

@Composable
private fun ThemeWatermark(theme: Theme, modifier: Modifier = Modifier) {
    val context = LocalContext.current
    val watermarkRes = themeWatermarkRes(theme) ?: return
    AsyncImage(
        model = ImageRequest.Builder(context)
            .data(androidResourceUri(context.packageName, watermarkRes))
            .decoderFactory(SvgDecoder.Factory())
            .crossfade(false)
            .build(),
        contentDescription = null,
        modifier = modifier,
        contentScale = ContentScale.Fit
    )
}

@RawRes
private fun themeWatermarkRes(theme: Theme): Int? = when (theme) {
    Theme.Piplup -> CoreR.raw.piplup
    Theme.PiplupAmoled -> CoreR.raw.piplup
    Theme.Rayquaza -> CoreR.raw.rayquaza
    Theme.Zapdos -> CoreR.raw.zapdos
    Theme.Charmeleon -> CoreR.raw.charmeleon
    Theme.Mew -> CoreR.raw.mew
    Theme.Salamence -> CoreR.raw.salamence
    Theme.Fraxure -> CoreR.raw.fraxure
    Theme.Default -> CoreR.raw.dynamic_default
    Theme.Custom -> CoreR.raw.custom_pokeball
}

private fun androidResourceUri(packageName: String, @RawRes resId: Int): String {
    return "android.resource://$packageName/$resId"
}
