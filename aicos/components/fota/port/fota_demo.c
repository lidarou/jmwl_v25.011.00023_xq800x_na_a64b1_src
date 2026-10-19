/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07b95e
#define LOG_MODULE    AMOD_FOTA

static void fota_dl_callback(void *user_data, int event, u32 data)
{
    FLOEX(0x05ebe5, "d", "fota do callback, event=%d\n", event);
}

static int fota_start(int argc, char **argv)
{
    struct fota_input in;

    int ret = 0;

    /* TODO: parse input param */
    in.crt = NULL;
    in.url[0] = 0;

    ret = fota_start_download(&in, fota_dl_callback, argv[0]);

    return ret;
}
CONSOLE_EXPORT_CMD(fota_start, fota_start, fota_start test);

static int fota_stop(int argc, char **argv)
{
    return fota_stop_download();
}
CONSOLE_EXPORT_CMD(fota_stop, fota_stop, fota_stop test);

