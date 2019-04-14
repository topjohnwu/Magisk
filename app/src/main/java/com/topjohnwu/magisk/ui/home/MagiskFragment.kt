package com.topjohnwu.magisk.ui.home

import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.utils.Event
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.magisk.view.dialogs.ManagerInstallDialog
import com.topjohnwu.magisk.view.dialogs.UninstallDialog
import org.koin.androidx.viewmodel.ext.android.viewModel
import com.topjohnwu.magisk.ui.base.MagiskFragment as NewMagiskFragment

class MagiskFragment : NewMagiskFragment<HomeViewModel, com.topjohnwu.magisk.databinding.FragmentMagiskBinding>() {

    /*@BindView(R.id.swipeRefreshLayout)
    internal var mSwipeRefreshLayout: SwipeRefreshLayout? = null
    @BindView(R.id.linearLayout)
    internal var root: LinearLayout? = null

    @BindView(R.id.install_option_card)
    internal var installOptionCard: CardView? = null
    @BindView(R.id.keep_force_enc)
    internal var keepEncChkbox: CheckBox? = null
    @BindView(R.id.keep_verity)
    internal var keepVerityChkbox: CheckBox? = null
    @BindView(R.id.install_option_expand)
    internal var optionExpandLayout: ViewGroup? = null
    @BindView(R.id.arrow)
    internal var arrow: ImageView? = null

    @BindView(R.id.uninstall_button)
    internal var uninstallButton: CardView? = null

    @BindColor(R.color.red500)
    internal var colorBad: Int = 0
    @BindColor(R.color.green500)
    internal var colorOK: Int = 0
    @BindColor(R.color.yellow500)
    internal var colorWarn: Int = 0
    @BindColor(R.color.green500)
    internal var colorNeutral: Int = 0
    @BindColor(R.color.blue500)
    internal var colorInfo: Int = 0*/

    /*private var magisk: UpdateCardHolder? = null
    private var manager: UpdateCardHolder? = null
    private var safetyNet: SafetyNet? = null
    private var transition: Transition? = null
    private var optionExpand: Expandable? = null*/

    override val layoutRes: Int = R.layout.fragment_magisk
    override val viewModel: HomeViewModel by viewModel()

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is OpenLinkEvent -> openLink(event.url)
            is ManagerInstallEvent -> installManager()
            is MagiskInstallEvent -> installMagisk()
            is UninstallEvent -> uninstall()
            is ManagerChangelogEvent -> changelogManager()
        }
    }

    private fun installMagisk() {
        // Show Manager update first
        if (Config.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
            ManagerInstallDialog(requireActivity()).show()
            return
        }
        //FIXME dialog requires old base
        //MagiskInstallDialog(requireActivity()).show()
    }

    private fun installManager() = ManagerInstallDialog(requireActivity()).show()

    private fun uninstall() {
        UninstallDialog(requireActivity()).show()
    }

    private fun changelogManager() {
        MarkDownWindow.show(
            requireActivity(), null,
            resources.openRawResource(R.raw.changelog)
        )
    }

    /*override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val v = inflater.inflate(R.layout.fragment_magisk, container, false)
        requireActivity().setTitle(R.string.magisk)

        //safetyNet = SafetyNet(v)

        *//*transition = TransitionSet()
            .setOrdering(TransitionSet.ORDERING_TOGETHER)
            .addTransition(Fade(Fade.OUT))
            .addTransition(ChangeBounds())
            .addTransition(Fade(Fade.IN))*//*

        updateUI()
        return v
    }*/

    private fun onRefresh() {
        /*mSwipeRefreshLayout!!.isRefreshing = false
        TransitionManager.beginDelayedTransition(root!!, transition)
        safetyNet!!.reset()
        magisk!!.reset()
        manager!!.reset()*/

        Config.loadMagiskInfo()
        updateUI()

        //FIXME requires old base
        /*Event.reset(this)
        Config.remoteMagiskVersionString = null
        Config.remoteMagiskVersionCode = -1*/

        shownDialog = false

        // Trigger state check
        /*if (Networking.checkNetworkStatus(app)) {
            CheckUpdates.check()
        }*/
    }

    private fun getListeningEvents(): IntArray {
        return intArrayOf(Event.UPDATE_CHECK_DONE)
    }

    private fun onEvent(event: Int) {
        updateCheckUI()
    }

    private fun updateUI() {
        /*(requireActivity() as MainActivity).checkHideSection()
        val image: Int
        val color: Int
        val status: String
        if (Config.magiskVersionCode < 0) {
            color = colorBad
            image = R.drawable.ic_cancel
            status = getString(R.string.magisk_version_error)
            magisk!!.status.text = status
            magisk!!.currentVersion.visibility = View.GONE
        } else {
            color = colorOK
            image = R.drawable.ic_check_circle
            status = getString(R.string.magisk)
            magisk!!.currentVersion.text = getString(
                R.string.current_installed,
                String.format(
                    Locale.US, "v%s (%d)",
                    Config.magiskVersionString, Config.magiskVersionCode
                )
            )
        }
        magisk!!.statusIcon.setColorFilter(color)
        magisk!!.statusIcon.setImageResource(image)

        manager!!.statusIcon.setColorFilter(colorOK)
        manager!!.statusIcon.setImageResource(R.drawable.ic_check_circle)
        manager!!.currentVersion.text = getString(
            R.string.current_installed,
            String.format(
                Locale.US, "v%s (%d)",
                BuildConfig.VERSION_NAME, BuildConfig.VERSION_CODE
            )
        )

        if (!Networking.checkNetworkStatus(app)) {
            // No network, updateCheckUI will not be triggered
            magisk!!.status.text = status
            manager!!.status.setText(R.string.app_name)
            magisk!!.setValid(false)
            manager!!.setValid(false)
        }*/
    }

    private fun updateCheckUI() {
        /*var image: Int
        var color: Int
        var status: String
        var button = ""


        if (Config.remoteMagiskVersionCode < 0) {
            color = colorNeutral
            image = R.drawable.ic_help
            status = getString(R.string.invalid_update_channel)
        } else {
            magisk!!.latestVersion.text = getString(
                R.string.latest_version,
                String.format(
                    Locale.US, "v%s (%d)",
                    Config.remoteMagiskVersionString, Config.remoteMagiskVersionCode
                )
            )
            if (Config.remoteMagiskVersionCode > Config.magiskVersionCode) {
                color = colorInfo
                image = R.drawable.ic_update
                status = getString(R.string.magisk_update_title)
                button = getString(R.string.update)
            } else {
                color = colorOK
                image = R.drawable.ic_check_circle
                status = getString(R.string.magisk_up_to_date)
                button = getString(R.string.install)
            }
        }
        if (Config.magiskVersionCode > 0) {
            // Only override status if Magisk is installed
            magisk!!.statusIcon.setImageResource(image)
            magisk!!.statusIcon.setColorFilter(color)
            magisk!!.status.text = status
            magisk!!.install.text = button
        }

        if (Config.remoteManagerVersionCode < 0) {
            color = colorNeutral
            image = R.drawable.ic_help
            status = getString(R.string.invalid_update_channel)
        } else {
            manager!!.latestVersion.text = getString(
                R.string.latest_version,
                String.format(
                    Locale.US, "v%s (%d)",
                    Config.remoteManagerVersionString, Config.remoteManagerVersionCode
                )
            )
            if (Config.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
                color = colorInfo
                image = R.drawable.ic_update
                status = getString(R.string.manager_update_title)
                manager!!.install.setText(R.string.update)
            } else {
                color = colorOK
                image = R.drawable.ic_check_circle
                status = getString(R.string.manager_up_to_date)
                manager!!.install.setText(R.string.install)
            }
        }
        manager!!.statusIcon.setImageResource(image)
        manager!!.statusIcon.setColorFilter(color)
        manager!!.status.text = status

        magisk!!.setValid(Config.remoteMagiskVersionCode > 0)
        manager!!.setValid(Config.remoteManagerVersionCode > 0)

        if (Config.remoteMagiskVersionCode < 0) {
            // Hide install related components
            installOptionCard!!.visibility = View.GONE
            uninstallButton!!.visibility = View.GONE
        } else {
            // Show install related components
            installOptionCard!!.visibility = View.VISIBLE
            uninstallButton!!.visibility = if (Shell.rootAccess()) View.VISIBLE else View.GONE
        }

        if (!shownDialog && Config.magiskVersionCode > 0 &&
            !Shell.su("env_check").exec().isSuccess
        ) {
            shownDialog = true
            EnvFixDialog(requireActivity()).show()
        }*/
    }

    companion object {

        private var shownDialog = false
    }
}

