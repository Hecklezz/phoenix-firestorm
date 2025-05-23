; First is default
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Japanese.nlf"

; Language selection dialog
LangString InstallerLanguageTitle  ${LANG_JAPANESE} "インストーラの言語"
LangString SelectInstallerLanguage  ${LANG_JAPANESE} "インストーラの言語を選択してください。"

; subtitle on license text caption
LangString LicenseSubTitleUpdate ${LANG_JAPANESE} "のアップデート"
LangString LicenseSubTitleSetup ${LANG_JAPANESE} "のセットアップ"

LangString MULTIUSER_TEXT_INSTALLMODE_TITLE ${LANG_JAPANESE} "インストールモード"
LangString MULTIUSER_TEXT_INSTALLMODE_SUBTITLE ${LANG_JAPANESE} "全てのユーザにインストール（要管理者権限）しますか？それとも現在のユーザだけにインストールしますか？"
LangString MULTIUSER_INNERTEXT_INSTALLMODE_TOP ${LANG_JAPANESE} "このインストーラを管理者の権限で実行する場合、次のいずれかへのインストールを選択できます。（例） c:\Program File または、現在のユーザの AppData\Localフォルダ"
LangString MULTIUSER_INNERTEXT_INSTALLMODE_ALLUSERS ${LANG_JAPANESE} "全てのユーザにインストール"
LangString MULTIUSER_INNERTEXT_INSTALLMODE_CURRENTUSER ${LANG_JAPANESE} "現在のユーザだけにインストール"

; installation directory text
LangString DirectoryChooseTitle ${LANG_JAPANESE} "インストール先のディレクトリ"
LangString DirectoryChooseUpdate ${LANG_JAPANESE} "アップデートするFirestormのディレクトリを選択してください："
LangString DirectoryChooseSetup ${LANG_JAPANESE} "Firestormをインストールするディレクトリを選択してください："

LangString MUI_TEXT_DIRECTORY_TITLE ${LANG_JAPANESE} "インストール先のディレクトリ"
LangString MUI_TEXT_DIRECTORY_SUBTITLE ${LANG_JAPANESE} "Firestormをインストールするディレクトリを選択してください:"

LangString MUI_TEXT_INSTALLING_TITLE ${LANG_JAPANESE} "Firestormをインストールしています…"
LangString MUI_TEXT_INSTALLING_SUBTITLE ${LANG_JAPANESE} "$INSTDIRにFirestormをインストールしています。"

LangString MUI_TEXT_FINISH_TITLE ${LANG_JAPANESE} "Firestormのインストール"
LangString MUI_TEXT_FINISH_SUBTITLE ${LANG_JAPANESE} "$INSTDIRにFirestormビューアをインストールしました。"

LangString MUI_TEXT_ABORT_TITLE ${LANG_JAPANESE} "インストールが中止されました"
LangString MUI_TEXT_ABORT_SUBTITLE ${LANG_JAPANESE} "$INSTDIRにFirestormビューアをインストールしませんでした。"

; CheckStartupParams message box
LangString CheckStartupParamsMB ${LANG_JAPANESE} "プログラム「$INSTNAME」が見つかりません。サイレントアップデートに失敗しました。"

; installation success dialog
LangString InstSuccesssQuestion ${LANG_JAPANESE} "Firestormを開始してもよろしいですか？"

; remove old NSIS version
LangString RemoveOldNSISVersion ${LANG_JAPANESE} "古いバージョン情報をチェックしています…"

; check windows version
LangString CheckWindowsVersionDP ${LANG_JAPANESE} "Windowsのバージョン情報をチェックしています…"
LangString CheckWindowsVersionMB ${LANG_JAPANESE} "FirestormはWindows Vista SP2以降のみをサポートしています。"
LangString CheckWindowsServPackMB ${LANG_JAPANESE} "Firestormは、オペレーティング システムの最新のサービスパックで実行することをお勧めします。$\nこれにより、プログラムのパフォーマンスと安定性が向上します。"
LangString UseLatestServPackDP ${LANG_JAPANESE} "最新のサービスパックをインストールするには、Windows Updateを使用してください。"

; checkifadministrator function (install)
LangString CheckAdministratorInstDP ${LANG_JAPANESE} "インストールのための権限をチェックしています…"
LangString CheckAdministratorInstMB ${LANG_JAPANESE} "Firestormをインストールするためには管理者権限が必要です。"

; checkifadministrator function (uninstall)
LangString CheckAdministratorUnInstDP ${LANG_JAPANESE} "アンインストールのための権限をチェックしています…"
LangString CheckAdministratorUnInstMB ${LANG_JAPANESE} "Firestormをアンインストールするためには管理者権限が必要です。"

; checkcpuflags
LangString MissingSSE2 ${LANG_JAPANESE} "このマシンには、Firestorm${VERSION_LONG}を実行するために必要なＳＳＥ２をサポートするＣＰＵが搭載されていない可能性があります。続けてもよろしいですか？"

; Extended cpu checks (AVX2)
LangString MissingAVX2 ${LANG_JAPANESE} "お使いのＣＰＵはＡＶＸ２命令をサポートしていません。旧バージョンを次のリンクからダウンロードしてください：%DLURL%-legacy-cpus/"
LangString AVX2Available ${LANG_JAPANESE} "お使いのＣＰＵはＡＶＸ２命令をサポートしています。より高いパフォーマンスを得るために、ＡＶＸ２最適化版を次のリンクからダウンロードできます：%DLURL%/ 今すぐダウンロードしますか？"
LangString AVX2OverrideConfirmation ${LANG_JAPANESE} "お使いのＰＣがＡＶＸ２最適化をサポートできると思われる場合は、インストールしても問題ありません。続行してもよろしいですか？"
LangString AVX2OverrideNote ${LANG_JAPANESE} "このままインストーラーをオーバーライドすると、起動直後にクラッシュする可能性のあるバージョンがインストールされます。このような場合は、すぐに標準ＣＰＵバージョンをインストールしてください。"

; closesecondlife function (install)
LangString CloseSecondLifeInstDP ${LANG_JAPANESE} "Firestormを終了しています…"
LangString CloseSecondLifeInstMB ${LANG_JAPANESE} "Firestormの起動中はインストールできません。直ちにFirestormを終了してインストールを開始する場合はＯＫボタンを押してください。キャンセルを押すと中止します。"
LangString CloseSecondLifeInstRM ${LANG_JAPANESE} "Firestormは以前のインストールからいくつかのファイルを削除できませんでした。"

; closesecondlife function (uninstall)
LangString CloseSecondLifeUnInstDP ${LANG_JAPANESE} "Firestormを終了しています…"
LangString CloseSecondLifeUnInstMB ${LANG_JAPANESE} "Firestormの起動中にアンインストールは出来ません。直ちにFirestormを終了してアンインストールを開始する場合はＯＫボタンを押してください。キャンセルを押すと中止します。"

; CheckNetworkConnection
LangString CheckNetworkConnectionDP ${LANG_JAPANESE} "ネットワークの接続を確認中…"

; error during installation
LangString ErrorSecondLifeInstallRetry ${LANG_JAPANESE} "Firestormインストーラーはインストール時に問題が発生しました。一部のファイルが正しくコピーされていない可能性があります。"
LangString ErrorSecondLifeInstallSupport ${LANG_JAPANESE} "https://www.firestormviewer.org/downloads/ からビューアを再インストールし、再インストール後も問題が解決しない場合は https://www.firestormviewer.org/support/ にお問い合わせください。"

; ask to remove user's data files
LangString RemoveDataFilesMB ${LANG_JAPANESE} "Documents and Settingsフォルダ内の設定とキャッシュファイルを削除しますか？"

; delete program files
LangString DeleteProgramFilesMB ${LANG_JAPANESE} "Firestormのディレクトリには、まだファイルが残されています。$\n$INSTDIR$\nにあなたが作成、または移動させたファイルがある可能性があります。全て削除しますか？ "

; uninstall text
LangString UninstallTextMsg ${LANG_JAPANESE} "Firestorm${VERSION_LONG}をアンインストールします。"

; ask to remove registry keys that still might be needed by other viewers that are installed
LangString DeleteRegistryKeysMB ${LANG_JAPANESE} "アプリケーションのレジストリキーを削除してもよろしいですか？$\n$\nFirestormの他のバージョンがインストールされている場合は、レジストリキーを保持することをお勧めします。"

; <FS:Ansariel> Ask to create protocol handler registry entries
LangString CreateUrlRegistryEntries ${LANG_JAPANESE} "Firestormをバーチャルワールドのプロトコルのデフォルトハンドラーとして登録しますか？$\n$\nFirestormの他のバージョンがインストールされている場合は、既存のレジストリキーが上書きされます。"

; <FS:Ansariel> Optional start menu entry
LangString CreateStartMenuEntry ${LANG_JAPANESE} "スタートメニューにエントリを作成しますか？"

; <FS:Ansariel> Application name suffix for OpenSim variant
LangString ForOpenSimSuffix ${LANG_JAPANESE} "for OpenSimulator"

LangString DeleteDocumentAndSettingsDP ${LANG_JAPANESE} "Documents and Settingsフォルダ内のファイルを削除しています。"
LangString UnChatlogsNoticeMB ${LANG_JAPANESE} "このアンインストールでは、Firestormチャットログやその他のプライベートファイルは削除されません。自分で削除したい場合は、ユーザのアプリケーションデータフォルダ内のFirestormフォルダを削除してください。"
LangString UnRemovePasswordsDP ${LANG_JAPANESE} "Firestormに保存されたパスワードを削除します。"

LangString MUI_TEXT_LICENSE_TITLE ${LANG_JAPANESE} "Vivox音声システムライセンス契約"
LangString MUI_TEXT_LICENSE_SUBTITLE ${LANG_JAPANESE} "Vivox Voice Systemコンポーネントの追加ライセンス契約です。"
LangString MUI_INNERTEXT_LICENSE_TOP ${LANG_JAPANESE} "インストールを続行する前に、次のライセンス契約をよくお読みください："
LangString MUI_INNERTEXT_LICENSE_BOTTOM ${LANG_JAPANESE} "インストールを続行するには、ライセンス条項に同意する必要があります。"
