typedef enum
{
  STATE_NORMAL = 0,
  STATE_READY,
  STATE_DOWNLOADING,
  STATE_DOWNLOADED,
  STATE_INSTALLING,
  STATE_INSTALLED,
  STATE_CANCEL,
  STATE_CLOSE,
  STATE_ERROR,
  N_STATE
} InstallState;

#define HTOOLKIT_CHECK "htoolkit-check"
#define HTOOLKIT_SCRIPT "htoolkit-install"
#define OUT_PATH "/var/tmp/htoolkit"
#define HTOOLKIT_PATH1 "/etc/lsb-release"
#define HTOOLKIT_PATH2 "/etc/gooroom/info"
#define HTOOLKIT1 "Gooroom"
#define HTOOLKIT2 "Hancom"
