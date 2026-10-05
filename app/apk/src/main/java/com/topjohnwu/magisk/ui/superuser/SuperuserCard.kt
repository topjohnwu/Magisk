package com.topjohnwu.magisk.ui.superuser

import android.graphics.drawable.Drawable
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Badge
import androidx.compose.material3.Card
import androidx.compose.material3.CardColors
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.ListItem
import androidx.compose.material3.ListItemDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.semantics.role
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.google.accompanist.drawablepainter.rememberDrawablePainter
import com.topjohnwu.magisk.core.R as CoreR

@Composable
internal fun SharedUidBadge(modifier: Modifier = Modifier) {
    Badge(
        modifier = modifier,
        containerColor = MaterialTheme.colorScheme.secondaryContainer,
        contentColor = MaterialTheme.colorScheme.onSecondaryContainer,
    ) {
        Text(
            text = stringResource(CoreR.string.shared_uid),
            style = MaterialTheme.typography.labelSmall,
            fontWeight = FontWeight.SemiBold,
            modifier = Modifier.padding(horizontal = 2.dp)
        )
    }
}

@Composable
fun SuperuserAppListItem(
    icon: Drawable,
    title: String,
    packageName: String,
    isSharedUid: Boolean,
    modifier: Modifier = Modifier,
    contentDescription: String? = null,
    trailingContent: @Composable (() -> Unit)? = null,
) {
    ListItem(
        modifier = modifier,
        leadingContent = {
            Image(
                painter = rememberDrawablePainter(icon),
                contentDescription = null,
                modifier = Modifier.size(40.dp)
            )
        },
        headlineContent = {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(
                    text = title,
                    style = MaterialTheme.typography.titleMedium,
                    modifier = Modifier.weight(1f, fill = false),
                )
                if (isSharedUid) {
                    Spacer(Modifier.width(6.dp))
                    SharedUidBadge()
                }
            }
        },
        supportingContent = {
            Text(
                text = packageName,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        },
        trailingContent = trailingContent,
        colors = ListItemDefaults.colors(containerColor = Color.Transparent)
    )
}

@Composable
fun SuperuserAppCard(
    icon: Drawable,
    title: String,
    packageName: String,
    isSharedUid: Boolean,
    modifier: Modifier = Modifier,
    onClick: (() -> Unit)? = null,
    checked: Boolean? = null,
    onCheckedChange: ((Boolean) -> Unit)? = null,
    enabled: Boolean = true,
    shape: Shape = RoundedCornerShape(20.dp),
    colors: CardColors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow),
    contentDescription: String? = null,
    trailingContent: @Composable (() -> Unit)? = null,
) {
    val itemContent = @Composable {
        SuperuserAppListItem(
            icon = icon,
            title = title,
            packageName = packageName,
            isSharedUid = isSharedUid,
            contentDescription = contentDescription,
            trailingContent = trailingContent,
        )
    }

    if (checked != null && onCheckedChange != null) {
        Surface(
            checked = checked,
            onCheckedChange = onCheckedChange,
            modifier = Modifier
                .fillMaxWidth()
                .semantics { role = Role.Checkbox }
                .then(modifier),
            enabled = enabled,
            shape = shape,
            color = if (enabled) colors.containerColor else colors.disabledContainerColor,
            contentColor = if (enabled) colors.contentColor else colors.disabledContentColor,
            content = itemContent,
        )
    } else if (onClick != null) {
        Card(
            onClick = onClick,
            modifier = Modifier
                .fillMaxWidth()
                .then(modifier),
            enabled = enabled,
            shape = shape,
            colors = colors,
            content = { itemContent() },
        )
    } else {
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .then(modifier),
            shape = shape,
            colors = colors,
            content = { itemContent() },
        )
    }
}
