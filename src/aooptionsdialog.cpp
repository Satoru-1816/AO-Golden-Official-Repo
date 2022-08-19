#include "aooptionsdialog.h"
#include "aoapplication.h"
#include "courtroom.h"
#include "lobby.h"
#include "bass.h"
#include "networkmanager.h"
#include "options.h"
#include "aoapplication.h"

#include <QUiLoader>

#include <QFileDialog>
#include <QString>
#include <QVBoxLayout>
#include <QCheckBox>

AOOptionsDialog::AOOptionsDialog(QWidget *parent, AOApplication *p_ao_app)
    : QWidget(parent)
{
  ao_app = p_ao_app;

  QUiLoader l_loader(this);
  QFile l_uiFile(":/resource/ui/optionsdialogue.ui");
  if (!l_uiFile.open(QFile::ReadOnly)) {
    qWarning() << "Unable to open file " << l_uiFile.fileName();
    return;
  }

  l_settings_widget = l_loader.load(&l_uiFile, this);

  auto l_layout = new QVBoxLayout(this);
  l_layout->addWidget(l_settings_widget);

  FROM_UI(QDialogButtonBox, settings_buttons);

  connect(ui_settings_buttons, &QDialogButtonBox::accepted, this,
                   &AOOptionsDialog::save_pressed);
  connect(ui_settings_buttons, &QDialogButtonBox::rejected, this,
                   &AOOptionsDialog::discard_pressed);
  connect(ui_settings_buttons, &QDialogButtonBox::clicked, this,
                   &AOOptionsDialog::button_clicked);

  FROM_UI(QComboBox, theme_combobox)
  registerOption<QComboBox, QString>("theme_combobox", &Options::theme, &Options::setTheme);

  QSet<QString> themes;
  QStringList bases = Options::options->mountpaths();
  bases.push_front(ao_app->get_base_path());
  for (const QString &base : bases) {
    QDirIterator it(base + "/themes", QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
      QString actualname = QDir(it.next()).dirName();
      if (!themes.contains(actualname)) {
        ui_theme_combobox->addItem(actualname);
        themes.insert(actualname);
      }
    }
  }

  FROM_UI(QComboBox, subtheme_combobox)
  connect(ui_theme_combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                   &AOOptionsDialog::theme_changed);
  registerOption<QComboBox, QString>("subtheme_combobox", &Options::subTheme, &Options::setSubTheme);

  QDirIterator it2(ao_app->get_real_path(ao_app->get_theme_path("")), QDir::Dirs,
                  QDirIterator::NoIteratorFlags);
  while (it2.hasNext()) {
    QString actualname = QDir(it2.next()).dirName();
    if (actualname != "." && actualname != ".." && actualname.toLower() != "server" && actualname.toLower() != "default" && actualname.toLower() != "effects" && actualname.toLower() != "misc") {
      ui_subtheme_combobox->addItem(actualname);
    }
  }

  FROM_UI(QPushButton, theme_reload_button)
  connect(ui_theme_reload_button, &QPushButton::clicked, this,
          &AOOptionsDialog::on_reload_theme_clicked);

  FROM_UI(QPushButton, theme_folder_button)
  connect(ui_theme_folder_button, &QPushButton::clicked, this,
    [=] {
      QString p_path = ao_app->get_real_path(ao_app->get_theme_path("", ui_theme_combobox->itemText(ui_theme_combobox->currentIndex())));
      if (!dir_exists(p_path)) {
        return;
      }
      QDesktopServices::openUrl(QUrl::fromLocalFile(p_path));
    }
  );

  FROM_UI(QCheckBox, animated_theme_cb)
  registerOption<QCheckBox, bool>("animated_theme_cb", &Options::animatedThemeEnabled, &Options::setAnimatedThemeEnabled);

  FROM_UI(QSpinBox, stay_time_spinbox)
  registerOption<QSpinBox, int>("stay_time_spinbox", &Options::textStayTime, &Options::setTextStayTime);

  FROM_UI(QCheckBox, instant_objection_cb)
  registerOption<QCheckBox, bool>("instant_objection_cb", &Options::objectionSkipQueueEnabled, &Options::setObjectionSkipQueueEnabled);

  FROM_UI(QSpinBox, text_crawl_spinbox)
  registerOption<QSpinBox, int>("text_crawl_spinbox", &Options::textCrawlSpeed, &Options::setTextCrawlSpeed);

  FROM_UI(QSpinBox, chat_ratelimit_spinbox)
  registerOption<QSpinBox, int>("chat_ratelimit_spinbox", &Options::chatRateLimit, &Options::setChatRateLimit);

  FROM_UI(QFrame, log_names_divider)

  FROM_UI(QLineEdit, username_textbox)
  registerOption<QLineEdit, QString>("username_textbox", &Options::username, &Options::setUsername);

  FROM_UI(QCheckBox, showname_cb)
  registerOption<QCheckBox, bool>("showname_cb", &Options::customShownameEnabled, &Options::setCustomShownameEnabled);

  FROM_UI(QLineEdit, default_showname_textbox);
  registerOption<QLineEdit, QString>("default_showname_textbox", &Options::shownameOnJoin, &Options::setShownameOnJoin);

  FROM_UI(QFrame, net_divider)

  FROM_UI(QLineEdit, ms_textbox)
  registerOption<QLineEdit, QString>("ms_textbox", &Options::alternativeMasterserver, &Options::setAlternativeMasterserver);

  FROM_UI(QCheckBox, discord_cb)
  registerOption<QCheckBox, bool>("discord_cb", &Options::discordEnabled, &Options::setDiscordEnabled);

  FROM_UI(QComboBox, language_combobox)
  registerOption<QComboBox, QString>("language_combobox", &Options::language, &Options::setLanguage);

  FROM_UI(QComboBox, scaling_combobox)
  registerOption<QComboBox, QString>("scaling_combobox", &Options::defaultScalingMode, &Options::setDefaultScalingMode);
  ui_scaling_combobox->addItem(tr("Pixel"), "fast");
  ui_scaling_combobox->addItem(tr("Smooth"), "smooth");

  FROM_UI(QCheckBox, shake_cb)
  registerOption<QCheckBox, bool>("shake_cb",  &Options::shakeEnabled, &Options::setShakeEnabled);

  FROM_UI(QCheckBox, effects_cb)
  registerOption<QCheckBox, bool>("effects_cb", &Options::effectsEnabled, &Options::setEffectsEnabled);

  FROM_UI(QCheckBox, framenetwork_cb)
  registerOption<QCheckBox, bool>("framenetwork_cb", &Options::networkedFrameSfxEnabled, &Options::setNetworkedFrameSfxEnabled);

  FROM_UI(QCheckBox, colorlog_cb)
  registerOption<QCheckBox, bool>("colorlog_cb", &Options::colorLogEnabled, &Options::setColorLogEnabled);

  FROM_UI(QCheckBox, stickysounds_cb)
  registerOption<QCheckBox, bool>("stickysounds_cb", &Options::clearSoundsDropdownOnPlayEnabled, &Options::setClearSoundsDropdownOnPlayEnabled);

  FROM_UI(QCheckBox, stickyeffects_cb)
  registerOption<QCheckBox, bool>("stickyeffects_cb", &Options::clearEffectsDropdownOnPlayEnabled, &Options::setClearEffectsDropdownOnPlayEnabled);

  FROM_UI(QCheckBox, stickypres_cb)
  registerOption<QCheckBox, bool>("stickypres_cb", &Options::clearPreOnPlayEnabled, &Options::setClearPreOnPlayEnabled);

  FROM_UI(QCheckBox, customchat_cb)
  registerOption<QCheckBox, bool>("customchat_cb", &Options::customChatboxEnabled, &Options::setCustomChatboxEnabled);

  FROM_UI(QCheckBox, sticker_cb)
  registerOption<QCheckBox, bool>("sticker_cb", &Options::characterStickerEnabled, &Options::setCharacterStickerEnabled);

  FROM_UI(QCheckBox, continuous_cb)
  registerOption<QCheckBox, bool>("continuous_cb", &Options::continuousPlaybackEnabled, &Options::setContinuousPlaybackEnabled);

  FROM_UI(QCheckBox, category_stop_cb)
  registerOption<QCheckBox, bool>("category_stop_cb", &Options::stopMusicOnCategoryEnabled, &Options::setStopMusicOnCategoryEnabled);

  FROM_UI(QCheckBox, sfx_on_idle_cb)
  registerOption<QCheckBox, bool>("sfx_on_idle_cb", &Options::SfxonIdle, &Options::setSfxOnIdle);

  FROM_UI(QCheckBox, evidence_double_click_cb)
  registerOption<QCheckBox, bool>("evidence_double_click_cb", &Options::evidenceDoubleClickEdit, &Options::setEvidenceDoubleClickEdit);

  FROM_UI(QPlainTextEdit, callwords_textbox)

  const QStringList temp_callword_list = options.callwords();
  for (const QString &callword : temp_callword_list) {
    ui_callwords_textbox->appendPlainText(callword);
  }


  update_values();
  /**

  ui_theme_label->setToolTip(
      tr("Sets the theme used in-game. If the new theme changes "
         "the lobby's look as well, you'll need to reload the "
         "lobby for the changes to take effect, such as by joining "
         "a server and leaving it."));

  ui_subtheme_label->setToolTip(
      tr("Sets a 'subtheme', which will stack on top of the current theme and replace anything it can."
         "Keep it at 'server' to let the server decide. Keep it at 'default' to keep it unchanging."));

  ui_theme_reload_button->setToolTip(
      tr("Refresh the theme and update all of the ui elements to match."));
  ui_gameplay_form->setWidget(row, QFormLayout::FieldRole, ui_theme_reload_button);

  ui_theme_folder_button->setToolTip(
      tr("Open the theme folder of the currently selected theme."));
  ui_gameplay_form->setWidget(row, QFormLayout::FieldRole, ui_theme_folder_button);

  ui_animated_theme_lbl->setToolTip(
      tr("If ticked, themes will be allowed to have animated elements."));

  ui_stay_time_lbl->setToolTip(tr(
      "Minimum amount of time (in miliseconds) an IC message must stay on screen before "
      "the next IC message is shown, acting as a 'queue'. Set to 0 to disable this behavior."));

  ui_text_crawl_lbl->setToolTip(tr(
      "Amount of time (in miliseconds) spent on each letter when the in-character text is being displayed."));

  ui_username_lbl->setToolTip(
      tr("Your OOC name will be automatically set to this value "
         "when you join a server."));

  ui_showname_lbl->setToolTip(
      tr("Gives the default value for the in-game 'Custom shownames' "
         "tickbox, which in turn determines whether the client should "
         "display custom in-character names."));

  ui_default_showname_lbl->setToolTip(
              tr("Your showname will be automatically set to this value "
                 "when you join a server."));

  ui_ms_lbl->setToolTip(
      tr("Overrides the base URL to retrieve server information from."));

  ui_discord_lbl->setToolTip(
      tr("Allows others on Discord to see what server you are in, "
         "what character are you playing, and how long you have "
         "been playing for."));

  ui_language_label->setToolTip(
      tr("Sets the language if you don't want to use your system language."));
  ui_gameplay_form->setWidget(row, QFormLayout::LabelRole, ui_language_label);

  ui_scaling_label->setToolTip(
        tr("Sets the default scaling method, if there is not one already defined "
           "specifically for the character."));

  ui_shake_lbl->setToolTip(
      tr("Allows screenshaking. Disable this if you have concerns or issues "
         "with photosensitivity and/or seizures."));

  ui_effects_lbl->setToolTip(
      tr("Allows screen effects. Disable this if you have concerns or issues "
         "with photosensitivity and/or seizures."));

  ui_framenetwork_lbl->setToolTip(tr(
      "Send screen-shaking, flashes and sounds as defined in the char.ini over "
      "the network. Only works for servers that support this functionality."));

  ui_colorlog_lbl->setToolTip(
      tr("Use the markup colors in the server IC chatlog."));

  ui_stickysounds_lbl->setToolTip(
      tr("Turn this on to prevent the sound dropdown from clearing the sound "
         "after playing it."));

  ui_stickyeffects_lbl->setToolTip(
      tr("Turn this on to prevent the effects dropdown from clearing the "
         "effect after playing it."));

  ui_stickypres_lbl->setToolTip(
      tr("Turn this on to prevent preanimation checkbox from clearing after "
         "playing the emote."));

  ui_customchat_lbl->setToolTip(
      tr("Turn this on to allow characters to define their own "
         "custom chat box designs."));

  ui_sticker_lbl->setToolTip(
      tr("Turn this on to allow characters to define their own "
         "stickers (unique images that show up over the chatbox - like avatars or shownames)."));

  ui_continuous_lbl->setToolTip(
      tr("Whether or not to resume playing animations from where they left off. Turning off might reduce lag."));

  ui_category_stop_lbl->setToolTip(
      tr("Stop music when double-clicking a category. If this is disabled, use the right-click context menu to stop music."));

  ui_sfx_on_idle_lbl->setToolTip(
      tr("If the SFX dropdown has an SFX selected, send the custom SFX alongside the message even if Preanim is OFF."));

  ui_evidence_double_click_lbl->setToolTip(
      tr("If ticked, Evidence needs a double-click to view rather than a single click."));

  ui_callwords_explain_lbl->setText(
      tr("<html><head/><body>Enter as many callwords as you would like. These "
         "are case insensitive. Make sure to leave every callword in its own "
         "line!<br>Do not leave a line with a space at the end -- you will be "
         "alerted everytime someone uses a space in their "
         "messages.</body></html>"));

  // The audio tab.
  ui_audio_tab = new QWidget(this);
  ui_settings_tabs->addTab(ui_audio_tab, tr("Audio"));

  ui_audio_widget = new QWidget(ui_audio_tab);
  ui_audio_widget->setGeometry(QRect(10, 10, 361, 211));

  ui_audio_layout = new QFormLayout(ui_audio_widget);
  ui_audio_layout->setLabelAlignment(Qt::AlignLeading | Qt::AlignLeft |
                                     Qt::AlignVCenter);
  ui_audio_layout->setFormAlignment(Qt::AlignLeading | Qt::AlignLeft |
                                    Qt::AlignTop);
  ui_audio_layout->setContentsMargins(0, 0, 0, 0);
  row = 0;

  ui_audio_device_lbl = new QLabel(ui_audio_widget);
  ui_audio_device_lbl->setText(tr("Audio device:"));
  ui_audio_device_lbl->setToolTip(tr("Sets the audio device for all sounds."));

  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_audio_device_lbl);

  ui_audio_device_combobox = new QComboBox(ui_audio_widget);

  // Let's fill out the combobox with the available audio devices. Or don't if
  // there is no audio
  int a = 0;
  if (needs_default_audiodev()) {

    ui_audio_device_combobox->addItem("default"); //TODO translate this without breaking the default audio device
  }
  BASS_DEVICEINFO info;
  for (a = 0; BASS_GetDeviceInfo(a, &info); a++) {
    ui_audio_device_combobox->addItem(info.name);
    if (Options::options->audioOutputDevice() == info.name)
      ui_audio_device_combobox->setCurrentIndex(
          ui_audio_device_combobox->count() - 1);
  }
  ui_audio_layout->setWidget(row, QFormLayout::FieldRole,
                             ui_audio_device_combobox);

  row += 1;
  ui_audio_volume_divider = new QFrame(ui_audio_widget);
  ui_audio_volume_divider->setFrameShape(QFrame::HLine);
  ui_audio_volume_divider->setFrameShadow(QFrame::Sunken);

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole,
                             ui_audio_volume_divider);

  row += 1;
  ui_music_volume_lbl = new QLabel(ui_audio_widget);
  ui_music_volume_lbl->setText(tr("Music:"));
  ui_music_volume_lbl->setToolTip(tr("Sets the music's default volume."));

  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_music_volume_lbl);

  ui_music_volume_spinbox = new QSpinBox(ui_audio_widget);
  ui_music_volume_spinbox->setMaximum(100);
  ui_music_volume_spinbox->setSuffix("%");

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole,
                             ui_music_volume_spinbox);

  row += 1;
  ui_sfx_volume_lbl = new QLabel(ui_audio_widget);
  ui_sfx_volume_lbl->setText(tr("SFX:"));
  ui_sfx_volume_lbl->setToolTip(
      tr("Sets the SFX's default volume. "
         "Interjections and actual sound effects count as 'SFX'."));
  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_sfx_volume_lbl);

  ui_sfx_volume_spinbox = new QSpinBox(ui_audio_widget);
  ui_sfx_volume_spinbox->setMaximum(100);
  ui_sfx_volume_spinbox->setSuffix("%");

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole,
                             ui_sfx_volume_spinbox);

  row += 1;
  ui_blips_volume_lbl = new QLabel(ui_audio_widget);
  ui_blips_volume_lbl->setText(tr("Blips:"));
  ui_blips_volume_lbl->setToolTip(
      tr("Sets the volume of the blips, the talking sound effects."));

  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_blips_volume_lbl);

  ui_blips_volume_spinbox = new QSpinBox(ui_audio_widget);
  ui_blips_volume_spinbox->setMaximum(100);
  ui_blips_volume_spinbox->setSuffix("%");

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole,
                             ui_blips_volume_spinbox);

  row += 1;
  ui_suppress_audio_lbl = new QLabel(ui_audio_widget);
  ui_suppress_audio_lbl->setText(tr("Suppress Audio:"));
  ui_suppress_audio_lbl->setToolTip(
      tr("How much of the volume to suppress when client is not in focus."));

  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_suppress_audio_lbl);

  ui_suppress_audio_spinbox = new QSpinBox(ui_audio_widget);
  ui_suppress_audio_spinbox->setMaximum(100);
  ui_suppress_audio_spinbox->setSuffix("%");

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole,
                             ui_suppress_audio_spinbox);

  row += 1;
  ui_volume_blip_divider = new QFrame(ui_audio_widget);
  ui_volume_blip_divider->setFrameShape(QFrame::HLine);
  ui_volume_blip_divider->setFrameShadow(QFrame::Sunken);

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole,
                             ui_volume_blip_divider);

  row += 1;
  ui_bliprate_lbl = new QLabel(ui_audio_widget);
  ui_bliprate_lbl->setText(tr("Blip rate:"));
  ui_bliprate_lbl->setToolTip(
      tr("Sets the delay between playing the blip sounds."));

  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_bliprate_lbl);

  ui_bliprate_spinbox = new QSpinBox(ui_audio_widget);
  ui_bliprate_spinbox->setMinimum(0);
  ui_bliprate_spinbox->setToolTip(
      tr("Play a blip sound \"once per every X symbols\", where "
         "X is the blip rate. 0 plays a blip sound only once."));

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole, ui_bliprate_spinbox);

  row += 1;
  ui_blank_blips_lbl = new QLabel(ui_audio_widget);
  ui_blank_blips_lbl->setText(tr("Blank blips:"));
  ui_blank_blips_lbl->setToolTip(
      tr("If true, the game will play a blip sound even "
         "when a space is 'being said'."));

  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_blank_blips_lbl);

  ui_blank_blips_cb = new QCheckBox(ui_audio_widget);

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole, ui_blank_blips_cb);

  row += 1;
  ui_loopsfx_lbl = new QLabel(ui_audio_widget);
  ui_loopsfx_lbl->setText(tr("Enable Looping SFX:"));
  ui_loopsfx_lbl->setToolTip(tr("If true, the game will allow looping sound "
                                "effects to play on preanimations."));

  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_loopsfx_lbl);

  ui_loopsfx_cb = new QCheckBox(ui_audio_widget);

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole, ui_loopsfx_cb);

  row += 1;
  ui_objectmusic_lbl = new QLabel(ui_audio_widget);
  ui_objectmusic_lbl->setText(tr("Kill Music On Objection:"));
  ui_objectmusic_lbl->setToolTip(
      tr("If true, AO2 will ask the server to stop music when you use 'Objection!' "));

  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_objectmusic_lbl);

  ui_objectmusic_cb = new QCheckBox(ui_audio_widget);

  ui_audio_layout->setWidget(row, QFormLayout::FieldRole, ui_objectmusic_cb);

  row += 1;
  ui_disablestreams_lbl = new QLabel(ui_audio_widget);
  ui_disablestreams_lbl->setText(tr("Disable Music Streaming:"));
  ui_disablestreams_lbl->setToolTip(
      tr("If true, AO2 will not play any streamed audio and show that streaming is disabled."));
  ui_audio_layout->setWidget(row, QFormLayout::LabelRole, ui_disablestreams_lbl);

  ui_disablestreams_cb = new QCheckBox(ui_audio_widget);
  ui_audio_layout->setWidget(row, QFormLayout::FieldRole, ui_disablestreams_cb);

  // The casing tab!
  ui_casing_tab = new QWidget(this);
  ui_settings_tabs->addTab(ui_casing_tab, tr("Casing"));

  ui_casing_widget = new QWidget(ui_casing_tab);
  ui_casing_widget->setGeometry(QRect(10, 10, 361, 211));

  ui_casing_layout = new QFormLayout(ui_casing_widget);
  ui_casing_layout->setLabelAlignment(Qt::AlignLeading | Qt::AlignLeft |
                                      Qt::AlignVCenter);
  ui_casing_layout->setFormAlignment(Qt::AlignLeading | Qt::AlignLeft |
                                     Qt::AlignTop);
  ui_casing_layout->setContentsMargins(0, 0, 0, 0);
  row = 0;

  // -- SERVER SUPPORTS CASING

  ui_casing_supported_lbl = new QLabel(ui_casing_widget);
  if (ao_app->casing_alerts_supported)
    ui_casing_supported_lbl->setText(tr("This server supports case alerts."));
  else
    ui_casing_supported_lbl->setText(
        tr("This server does not support case alerts."));
  ui_casing_supported_lbl->setToolTip(tr("Pretty self-explanatory."));

  ui_casing_layout->setWidget(row, QFormLayout::FieldRole,
                              ui_casing_supported_lbl);

  // -- CASE ANNOUNCEMENTS

  row += 1;
  ui_casing_enabled_lbl = new QLabel(ui_casing_widget);
  ui_casing_enabled_lbl->setText(tr("Casing:"));
  ui_casing_enabled_lbl->setToolTip(
      tr("If checked, you will get alerts about case "
         "announcements."));

  ui_casing_layout->setWidget(row, QFormLayout::LabelRole,
                              ui_casing_enabled_lbl);

  ui_casing_enabled_cb = new QCheckBox(ui_casing_widget);

  ui_casing_layout->setWidget(row, QFormLayout::FieldRole,
                              ui_casing_enabled_cb);

  // -- DEFENSE ANNOUNCEMENTS

  row += 1;
  ui_casing_def_lbl = new QLabel(ui_casing_widget);
  ui_casing_def_lbl->setText(tr("Defense:"));
  ui_casing_def_lbl->setToolTip(tr("If checked, you will get alerts about case "
                                   "announcements if a defense spot is open."));

  ui_casing_layout->setWidget(row, QFormLayout::LabelRole, ui_casing_def_lbl);

  ui_casing_def_cb = new QCheckBox(ui_casing_widget);

  ui_casing_layout->setWidget(row, QFormLayout::FieldRole, ui_casing_def_cb);

  // -- PROSECUTOR ANNOUNCEMENTS

  row += 1;
  ui_casing_pro_lbl = new QLabel(ui_casing_widget);
  ui_casing_pro_lbl->setText(tr("Prosecution:"));
  ui_casing_pro_lbl->setToolTip(
      tr("If checked, you will get alerts about case "
         "announcements if a prosecutor spot is open."));

  ui_casing_layout->setWidget(row, QFormLayout::LabelRole, ui_casing_pro_lbl);

  ui_casing_pro_cb = new QCheckBox(ui_casing_widget);

  ui_casing_layout->setWidget(row, QFormLayout::FieldRole, ui_casing_pro_cb);

  // -- JUDGE ANNOUNCEMENTS

  row += 1;
  ui_casing_jud_lbl = new QLabel(ui_casing_widget);
  ui_casing_jud_lbl->setText(tr("Judge:"));
  ui_casing_jud_lbl->setToolTip(tr("If checked, you will get alerts about case "
                                   "announcements if the judge spot is open."));

  ui_casing_layout->setWidget(row, QFormLayout::LabelRole, ui_casing_jud_lbl);

  ui_casing_jud_cb = new QCheckBox(ui_casing_widget);

  ui_casing_layout->setWidget(row, QFormLayout::FieldRole, ui_casing_jud_cb);

  // -- JUROR ANNOUNCEMENTS

  row += 1;
  ui_casing_jur_lbl = new QLabel(ui_casing_widget);
  ui_casing_jur_lbl->setText(tr("Juror:"));
  ui_casing_jur_lbl->setToolTip(tr("If checked, you will get alerts about case "
                                   "announcements if a juror spot is open."));

  ui_casing_layout->setWidget(row, QFormLayout::LabelRole, ui_casing_jur_lbl);

  ui_casing_jur_cb = new QCheckBox(ui_casing_widget);

  ui_casing_layout->setWidget(row, QFormLayout::FieldRole, ui_casing_jur_cb);

  // -- STENO ANNOUNCEMENTS

  row += 1;
  ui_casing_steno_lbl = new QLabel(ui_casing_widget);
  ui_casing_steno_lbl->setText(tr("Stenographer:"));
  ui_casing_steno_lbl->setToolTip(
      tr("If checked, you will get alerts about case "
         "announcements if a stenographer spot is open."));

  ui_casing_layout->setWidget(row, QFormLayout::LabelRole, ui_casing_steno_lbl);

  ui_casing_steno_cb = new QCheckBox(ui_casing_widget);

  ui_casing_layout->setWidget(row, QFormLayout::FieldRole, ui_casing_steno_cb);

  // -- CM ANNOUNCEMENTS

  row += 1;
  ui_casing_cm_lbl = new QLabel(ui_casing_widget);
  ui_casing_cm_lbl->setText(tr("CM:"));
  ui_casing_cm_lbl->setToolTip(
      tr("If checked, you will appear amongst the potential "
         "CMs on the server."));

  ui_casing_layout->setWidget(row, QFormLayout::LabelRole, ui_casing_cm_lbl);

  ui_casing_cm_cb = new QCheckBox(ui_casing_widget);

  ui_casing_layout->setWidget(row, QFormLayout::FieldRole, ui_casing_cm_cb);

  // -- CM CASES ANNOUNCEMENTS

  row += 1;
  ui_casing_cm_cases_lbl = new QLabel(ui_casing_widget);
  ui_casing_cm_cases_lbl->setText(tr("Hosting cases:"));
  ui_casing_cm_cases_lbl->setToolTip(
      tr("If you're a CM, enter what cases you are "
         "willing to host."));

  ui_casing_layout->setWidget(row, QFormLayout::LabelRole,
                              ui_casing_cm_cases_lbl);

  ui_casing_cm_cases_textbox = new QLineEdit(ui_casing_widget);

  ui_casing_layout->setWidget(row, QFormLayout::FieldRole,
                              ui_casing_cm_cases_textbox);

  // Assets tab
  ui_assets_tab = new QWidget(this);
  ui_assets_tab_layout = new QVBoxLayout(ui_assets_tab);
  ui_assets_tab->setLayout(ui_assets_tab_layout);
  ui_settings_tabs->addTab(ui_assets_tab, tr("Assets"));

  ui_asset_lbl = new QLabel(ui_assets_tab);
  ui_asset_lbl->setText(
        tr("Add or remove base folders for use by assets. "
           "Base folders on the bottom are prioritized over those above them."));
  ui_asset_lbl->setWordWrap(true);
  ui_assets_tab_layout->addWidget(ui_asset_lbl);

  ui_mount_list = new QListWidget(ui_assets_tab);
  ui_assets_tab_layout->addWidget(ui_mount_list);

  ui_mount_buttons_layout = new QGridLayout(ui_assets_tab);
  ui_assets_tab_layout->addLayout(ui_mount_buttons_layout);

  QSizePolicy stretch_btns(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  stretch_btns.setHorizontalStretch(4);

  ui_mount_add = new QPushButton(tr("Add…"), ui_assets_tab);
  ui_mount_add->setSizePolicy(stretch_btns);
  ui_mount_buttons_layout->addWidget(ui_mount_add, 0, 0, 1, 1);
  connect(ui_mount_add, &QPushButton::clicked, this, [this] {
    QString path = QFileDialog::getExistingDirectory(this, tr("Select a base folder"),
                                                    QApplication::applicationDirPath(),
                                                    QFileDialog::ShowDirsOnly);
    if (path.isEmpty()) {
      return;
    }
    QDir dir(QApplication::applicationDirPath());
    QString relative = dir.relativeFilePath(path);
    if (!relative.contains("../")) {
      path = relative;
    }
    QListWidgetItem *dir_item = new QListWidgetItem(path);
    ui_mount_list->addItem(dir_item);
    ui_mount_list->setCurrentItem(dir_item);

    // quick hack to update buttons
    emit ui_mount_list->itemSelectionChanged();
  });

  ui_mount_remove = new QPushButton(tr("Remove"), ui_assets_tab);
  ui_mount_remove->setSizePolicy(stretch_btns);
  ui_mount_remove->setEnabled(false);
  ui_mount_buttons_layout->addWidget(ui_mount_remove, 0, 1, 1, 1);
  connect(ui_mount_remove, &QPushButton::clicked, this, [this] {
    auto selected = ui_mount_list->selectedItems();
    if (selected.isEmpty())
      return;
    delete selected[0];
    emit ui_mount_list->itemSelectionChanged();
    asset_cache_dirty = true;
  });

  auto *mount_buttons_spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                               QSizePolicy::Minimum);
  ui_mount_buttons_layout->addItem(mount_buttons_spacer, 0, 2, 1, 1);

  ui_mount_up = new QPushButton(tr("↑"), ui_assets_tab);
  ui_mount_up->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  ui_mount_up->setMaximumWidth(40);
  ui_mount_up->setEnabled(false);
  ui_mount_buttons_layout->addWidget(ui_mount_up, 0, 3, 1, 1);
  connect(ui_mount_up, &QPushButton::clicked, this, [this] {
    auto selected = ui_mount_list->selectedItems();
    if (selected.isEmpty())
      return;
    auto *item = selected[0];
    int row = ui_mount_list->row(item);
    ui_mount_list->takeItem(row);
    int new_row = qMax(1, row - 1);
    ui_mount_list->insertItem(new_row, item);
    ui_mount_list->setCurrentRow(new_row);
    asset_cache_dirty = true;
  });

  ui_mount_down = new QPushButton(tr("↓"), ui_assets_tab);
  ui_mount_down->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  ui_mount_down->setMaximumWidth(40);
  ui_mount_down->setEnabled(false);
  ui_mount_buttons_layout->addWidget(ui_mount_down, 0, 4, 1, 1);
  connect(ui_mount_down, &QPushButton::clicked, this, [this] {
    auto selected = ui_mount_list->selectedItems();
    if (selected.isEmpty())
      return;
    auto *item = selected[0];
    int row = ui_mount_list->row(item);
    ui_mount_list->takeItem(row);
    int new_row = qMin(ui_mount_list->count() + 1, row + 1);
    ui_mount_list->insertItem(new_row, item);
    ui_mount_list->setCurrentRow(new_row);
    asset_cache_dirty = true;
  });

  auto *mount_buttons_spacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                                 QSizePolicy::Minimum);
  ui_mount_buttons_layout->addItem(mount_buttons_spacer_2, 0, 5, 1, 1);

  ui_mount_clear_cache = new QPushButton(tr("Clear Cache"), ui_assets_tab);
  ui_mount_clear_cache->setToolTip(tr("Clears the lookup cache for assets. "
  "Use this when you have added an asset that takes precedence over another "
  "existing asset."));
  ui_mount_buttons_layout->addWidget(ui_mount_clear_cache, 0, 6, 1, 1);
  connect(ui_mount_clear_cache, &QPushButton::clicked, this, [this] {
    asset_cache_dirty = true;
    ui_mount_clear_cache->setEnabled(false);
  });

  connect(ui_mount_list, &QListWidget::itemSelectionChanged, this, [this] {
    auto selected_items = ui_mount_list->selectedItems();
    bool row_selected = !ui_mount_list->selectedItems().isEmpty();
    ui_mount_remove->setEnabled(row_selected);
    ui_mount_up->setEnabled(row_selected);
    ui_mount_down->setEnabled(row_selected);

    if (!row_selected)
      return;

    int row = ui_mount_list->row(selected_items[0]);
    if (row <= 1)
      ui_mount_up->setEnabled(false);
    if (row >= ui_mount_list->count() - 1)
      ui_mount_down->setEnabled(false);
  });

  // Logging tab
  ui_logging_tab = new QWidget(this);
  ui_settings_tabs->addTab(ui_logging_tab, tr("Logging"));
  ui_form_logging_widget = new QWidget(this);

  ui_logging_form = new QFormLayout(ui_form_logging_widget);
  ui_logging_form->setLabelAlignment(Qt::AlignLeading | Qt::AlignLeft |
                                     Qt::AlignVCenter);
  ui_logging_form->setFormAlignment(Qt::AlignLeading | Qt::AlignLeft |
                                    Qt::AlignTop);
  ui_logging_form->setContentsMargins(5, 5, 0, 0);
  ui_logging_form->setSpacing(4);
  row = 0;

  ui_downwards_lbl = new QLabel(ui_form_logging_widget);
  ui_downwards_lbl->setText(tr("Log goes downwards:"));
  ui_downwards_lbl->setToolTip(
      tr("If ticked, new messages will appear at "
         "the bottom (like the OOC chatlog). The traditional "
         "(AO1) behaviour is equivalent to this being unticked."));

  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_downwards_lbl);

  ui_downwards_cb = new QCheckBox(ui_form_logging_widget);

  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_downwards_cb);

  row += 1;
  ui_length_lbl = new QLabel(ui_form_logging_widget);
  ui_length_lbl->setText(tr("Log length:"));
  ui_length_lbl->setToolTip(tr(
      "The amount of message lines the IC chatlog will keep before "
      "deleting older message lines. A value of 0 or below counts as 'infinite'."));

  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_length_lbl);

  ui_length_spinbox = new QSpinBox(ui_form_logging_widget);
  ui_length_spinbox->setSuffix(" lines");
  ui_length_spinbox->setMaximum(10000);

  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_length_spinbox);

  row += 1;
  ui_log_newline_lbl = new QLabel(ui_form_logging_widget);
  ui_log_newline_lbl->setText(tr("Log newline:"));
  ui_log_newline_lbl->setToolTip(
      tr("If ticked, new messages will appear separated, "
         "with the message coming on the next line after the name. "
         "When unticked, it displays it as 'name: message'."));

  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_log_newline_lbl);

  ui_log_newline_cb = new QCheckBox(ui_form_logging_widget);

  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_log_newline_cb);

  row += 1;
  ui_log_margin_lbl = new QLabel(ui_form_logging_widget);
  ui_log_margin_lbl->setText(tr("Log margin:"));
  ui_log_margin_lbl->setToolTip(tr(
      "The distance in pixels between each entry in the IC log. "
      "Default: 0."));

  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_log_margin_lbl);

  ui_log_margin_spinbox = new QSpinBox(ui_form_logging_widget);
  ui_log_margin_spinbox->setSuffix(" px");
  ui_log_margin_spinbox->setMaximum(1000);

  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_log_margin_spinbox);

  row += 1;
  ui_log_timestamp_lbl = new QLabel(ui_form_logging_widget);
  ui_log_timestamp_lbl->setText(tr("Log timestamp:"));
  ui_log_timestamp_lbl->setToolTip(
      tr("If ticked, log will contain a timestamp in UTC before the name."));

  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_log_timestamp_lbl);

  ui_log_timestamp_cb = new QCheckBox(ui_form_logging_widget);

  connect(ui_log_timestamp_cb, &QCheckBox::stateChanged, this, &AOOptionsDialog::timestamp_cb_changed);

  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_log_timestamp_cb);

  row += 1;
  ui_log_timestamp_format_lbl = new QLabel(ui_form_logging_widget);
  ui_log_timestamp_format_lbl->setText(tr("Log timestamp format:\n") + QDateTime::currentDateTime().toString(Options::options->logTimestampFormat()));
  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_log_timestamp_format_lbl);

  ui_log_timestamp_format_combobox = new QComboBox(ui_form_logging_widget);
  ui_log_timestamp_format_combobox->setEditable(true);

  QString l_current_format = Options::options->logTimestampFormat();

  ui_log_timestamp_format_combobox->setCurrentText(l_current_format);
  ui_log_timestamp_format_combobox->addItem("h:mm:ss AP"); // 2:13:09 PM
  ui_log_timestamp_format_combobox->addItem("hh:mm:ss"); // 14:13:09
  ui_log_timestamp_format_combobox->addItem("h:mm AP"); // 2:13 PM
  ui_log_timestamp_format_combobox->addItem("hh:mm"); // 14:13

  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_log_timestamp_format_combobox);

  connect(ui_log_timestamp_format_combobox, &QComboBox::currentTextChanged, this, &AOOptionsDialog::on_timestamp_format_edited);

  if(!Options::options->logTimestampEnabled()) {
    ui_log_timestamp_format_combobox->setDisabled(true);
  }
  row += 1;
  ui_log_ic_actions_lbl = new QLabel(ui_form_logging_widget);
  ui_log_ic_actions_lbl->setText(tr("Log IC actions:"));
  ui_log_ic_actions_lbl->setToolTip(
      tr("If ticked, log will show IC actions such as shouting and presenting evidence."));

  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_log_ic_actions_lbl);

  ui_log_ic_actions_cb = new QCheckBox(ui_form_logging_widget);

  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_log_ic_actions_cb);

  row += 1;
  ui_desync_logs_lbl = new QLabel(ui_form_logging_widget);
  ui_desync_logs_lbl->setText(tr("Desynchronize IC Logs:"));
  ui_desync_logs_lbl->setToolTip(
      tr("If ticked, log will show messages as-received, while viewport will parse according to the queue (Text Stay Time)."));

  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_desync_logs_lbl);

  ui_desync_logs_cb = new QCheckBox(ui_form_logging_widget);

  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_desync_logs_cb);

  //Check whether mass logging is enabled
  row += 1;
  ui_log_text_lbl = new QLabel(ui_form_logging_widget);
  ui_log_text_lbl->setText(tr("Log to Text Files:"));
  ui_log_text_lbl->setToolTip(
      tr("Text logs of gameplay will be automatically written in the /logs folder."));
  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_log_text_lbl);

  ui_log_text_cb = new QCheckBox(ui_form_logging_widget);
  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_log_text_cb);

  row += 1;
  ui_log_demo_lbl = new QLabel(ui_form_logging_widget);
  ui_log_demo_lbl->setText(tr("Log to Demo Files:"));
  ui_log_demo_lbl->setToolTip(
      tr("Gameplay will be automatically recorded as demos in the /logs folder."));
  ui_logging_form->setWidget(row, QFormLayout::LabelRole, ui_log_demo_lbl);

  ui_log_demo_cb = new QCheckBox(ui_form_logging_widget);
  ui_logging_form->setWidget(row, QFormLayout::FieldRole, ui_log_demo_cb);

  // Finish logging tab
  QScrollArea *log_scroll = new QScrollArea(this);
  log_scroll->setWidget(ui_form_logging_widget);
  ui_logging_tab->setLayout(new QVBoxLayout);
  ui_logging_tab->layout()->addWidget(log_scroll);

  // Privacy tab
  ui_privacy_tab = new QWidget(this);
  ui_settings_tabs->addTab(ui_privacy_tab, tr("Privacy"));

  ui_privacy_layout = new QVBoxLayout(ui_privacy_tab);

  ui_privacy_optout_cb = new QCheckBox(ui_privacy_tab);
  ui_privacy_optout_cb->setText(tr("Do not include me in public player counts"));
  ui_privacy_layout->addWidget(ui_privacy_optout_cb);

  ui_privacy_separator = new QFrame(ui_privacy_tab);
  ui_privacy_separator->setObjectName(QString::fromUtf8("line"));
  ui_privacy_separator->setFrameShape(QFrame::HLine);
  ui_privacy_separator->setFrameShadow(QFrame::Sunken);
  ui_privacy_layout->addWidget(ui_privacy_separator);

  ui_privacy_policy = new QTextBrowser(ui_privacy_tab);
  QSizePolicy privacySizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  ui_privacy_policy->setSizePolicy(privacySizePolicy);
  ui_privacy_policy->setPlainText(tr("Getting privacy policy..."));
  ui_privacy_layout->addWidget(ui_privacy_policy);

  update_values();

  // When we're done, we should continue the updates!
  setUpdatesEnabled(true);

  */
}

template<>
void AOOptionsDialog::setWidgetData(QCheckBox *widget, const bool &value)
{
  widget->setChecked(value);
}

template<>
bool AOOptionsDialog::widgetData(QCheckBox *widget) const
{
  return widget->isChecked();
}

template<>
void AOOptionsDialog::setWidgetData(QLineEdit *widget, const QString &value)
{
  widget->setText(value);
}

template<>
QString AOOptionsDialog::widgetData(QLineEdit *widget) const
{
  return widget->text();
}

template<>
void AOOptionsDialog::setWidgetData(QLineEdit *widget, const uint16_t &value)
{
  widget->setText(QString::number(value));
}

template<>
uint16_t AOOptionsDialog::widgetData(QLineEdit *widget) const
{
  return widget->text().toUShort();
}

template<>
void AOOptionsDialog::setWidgetData(QPlainTextEdit *widget, const QStringList &value)
{
  widget->setPlainText(value.join('\n'));
}

template<>
QStringList AOOptionsDialog::widgetData(QPlainTextEdit *widget) const
{
  return widget->toPlainText().trimmed().split('\n');
}

template<>
void AOOptionsDialog::setWidgetData(QSpinBox *widget, const int &value)
{
  widget->setValue(value);
}

template<>
int AOOptionsDialog::widgetData(QSpinBox *widget) const
{
  return widget->value();
}

template<>
void AOOptionsDialog::setWidgetData(QComboBox *widget, const QString &value)
{
  for (auto i = 0; i < widget->count(); i++) {
    if (widget->itemText(i) == value) {
      widget->setCurrentIndex(i);
      return;
    }
  }
  qWarning() << "value" << value << "not found for widget" << widget->objectName();
}

template<>
QString AOOptionsDialog::widgetData(QComboBox *widget) const
{
  return widget->currentText();
}

template<typename T, typename V>
void AOOptionsDialog::registerOption(const QString &widgetName,
                                     V (Options::*getter)() const,
                                     void (Options::*setter)(V))
{
  auto *widget = findChild<T *>(widgetName);
  if (!widget) {
    qWarning() << "could not find widget" << widgetName;
    return;
  }

  OptionEntry entry;
  entry.load = [=] {
    setWidgetData<T, V>(widget, (options.*getter)());
  };
  entry.save = [=] {
    (options.*setter)(widgetData<T, V>(widget));
  };

  optionEntries.append(entry);
}

void AOOptionsDialog::update_values()
{
    for (const OptionEntry &entry : qAsConst(optionEntries))
      entry.load();
    this->hide();
}

void AOOptionsDialog::save_pressed()
{
    for (const OptionEntry &entry : qAsConst(optionEntries))
      entry.save();

    //TODO : Figure out a way to do this proper.
    options.setCallwords(ui_callwords_textbox->toPlainText());
    this->hide();
}

void AOOptionsDialog::discard_pressed() {
    // The .temp file exists, meaning we are trying to undo the user clicking on "Restore defaults" and bring back the old settings.
    if (QFile::exists(ao_app->get_base_path() + "config.temp")) {
        // Delete the QSettings object so it does not interfere with the file
        delete ao_app->configini;
        // Remove the current config.ini
        QFile::remove(ao_app->get_base_path() + "config.ini");
        // Rename .temp to .ini
        QFile::rename(ao_app->get_base_path() + "config.temp", ao_app->get_base_path() + "config.ini");
        // Recreate the QSettings object from the ini file, restoring the settings before the Options Dialog was opened..
        ao_app->configini =
            new QSettings(ao_app->get_base_path() + "config.ini", QSettings::IniFormat);
    }
    this->hide();
}

void AOOptionsDialog::button_clicked(QAbstractButton *button) {
    if (ui_settings_buttons->buttonRole(button) == QDialogButtonBox::ResetRole) {
        // Store the current settings as a .temp file
        QFile::rename(ao_app->get_base_path() + "config.ini", ao_app->get_base_path() + "config.temp");
        // Load up the default settings
        ao_app->configini->clear();
        // Update the values on the settings ui
        update_values();
    }
}

void AOOptionsDialog::on_reload_theme_clicked() {
    ao_app->configini->setValue("theme", ui_theme_combobox->currentText());
    ao_app->configini->setValue("subtheme", ui_subtheme_combobox->currentText());
    ao_app->configini->setValue("animated_theme", ui_animated_theme_cb->isChecked());
    if (ao_app->courtroom_constructed)
        ao_app->w_courtroom->on_reload_theme_clicked();
    if (ao_app->lobby_constructed)
        ao_app->w_lobby->set_widgets();
}

void AOOptionsDialog::theme_changed(int i) {
  ui_subtheme_combobox->clear();
  // Fill the combobox with the names of the themes.
  ui_subtheme_combobox->addItem("server");
  ui_subtheme_combobox->addItem("default");
  QDirIterator it(ao_app->get_real_path(ao_app->get_theme_path("", ui_theme_combobox->itemText(i))), QDir::Dirs,
                  QDirIterator::NoIteratorFlags);
  while (it.hasNext()) {
    QString actualname = QDir(it.next()).dirName();
    if (actualname != "." && actualname != ".." && actualname.toLower() != "server" && actualname.toLower() != "default" && actualname.toLower() != "effects" && actualname.toLower() != "misc")
      ui_subtheme_combobox->addItem(actualname);
  }

}

void AOOptionsDialog::on_timestamp_format_edited() { ui_log_timestamp_format_lbl->setText(tr("Log timestamp format:\n") + QDateTime::currentDateTime().toString(ui_log_timestamp_format_combobox->currentText())); }

void AOOptionsDialog::timestamp_cb_changed(int state) { ui_log_timestamp_format_combobox->setDisabled(state == 0); }

#if (defined(_WIN32) || defined(_WIN64))
bool AOOptionsDialog::needs_default_audiodev() { return true; }
#elif (defined(LINUX) || defined(__linux__))
bool AOOptionsDialog::needs_default_audiodev() { return false; }
#elif defined __APPLE__
bool AOOptionsDialog::needs_default_audiodev() { return true; }
#else
#error This operating system is not supported.
#endif
