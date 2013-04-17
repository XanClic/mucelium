#include "mruby.h"
#include "mruby/class.h"
#include "mruby/compile.h"
#include "mruby/data.h"

#include <ctype.h>
#include <limits.h>
#include <shm.h>
#include <string.h>
#include <vfs.h>


static void mrb_pipe_free(mrb_state *mrb, void *ptr)
{
    mrb_free(mrb, ptr);
}


static struct mrb_data_type mrb_pipe_type = { "Pipe", mrb_pipe_free };


static mrb_value f_create_pipe(mrb_state *mrb, mrb_value self)
{
    char *fname_raw;
    int fname_len, mode = O_RDONLY;

    mrb_get_args(mrb, "s|i", &fname_raw, &fname_len, &mode);

    char fname[fname_len + 1];
    memcpy(fname, fname_raw, fname_len); fname[fname_len] = 0;

    int p = create_pipe(fname, mode);

    if (p < 0)
        return mrb_nil_value();

    int *pp = mrb_malloc(mrb, sizeof(p));
    *pp = p;

    return mrb_obj_value(mrb_data_object_alloc(mrb, mrb_class_ptr(self), pp, &mrb_pipe_type));
}


static mrb_value f_from_fd(mrb_state *mrb, mrb_value self)
{
    int fd;

    mrb_get_args(mrb, "i", &fd);

    int *pp = mrb_malloc(mrb, sizeof(fd));
    *pp = fd;

    return mrb_obj_value(mrb_data_object_alloc(mrb, mrb_class_ptr(self), pp, &mrb_pipe_type));
}


static mrb_value f_stream_send(mrb_state *mrb, mrb_value self)
{
    char *data;
    int data_len, flags = O_BLOCKING;

    mrb_get_args(mrb, "s|i", &data, &data_len, &flags);

    return mrb_fixnum_value(stream_send(*(int *)DATA_PTR(self), data, data_len, flags));
}


static mrb_value f_stream_receive(mrb_state *mrb, mrb_value self)
{
    int length = -1, flags = O_BLOCKING;

    mrb_get_args(mrb, "|ii", &length, &flags);

    if (length < 0)
        length = pipe_get_flag(*(int *)DATA_PTR(self), F_PRESSURE);

    uintptr_t shm = shm_create(length);
    void *mapping = shm_open(shm, VMM_UR);

    length = stream_recv_shm(*(int *)DATA_PTR(self), shm, length, flags);

    mrb_value ret = mrb_str_new(mrb, mapping, length);

    shm_close(shm, mapping, true);

    return ret;
}


static mrb_value f_destroy_pipe(mrb_state *mrb, mrb_value self)
{
    int flags = 0;

    mrb_get_args(mrb, "|i", &flags);

    destroy_pipe(*(int *)DATA_PTR(self), flags);

    return mrb_nil_value();
}


static mrb_value f_duplicate_pipe(mrb_state *mrb, mrb_value self)
{
    int dest = 0;

    mrb_get_args(mrb, "|i", &dest);

    int p = duplicate_pipe(*(int *)DATA_PTR(self), dest);

    if (p < 0)
        return mrb_nil_value();

    int *pp = mrb_malloc(mrb, sizeof(p));
    *pp = p;

    return mrb_obj_value(mrb_data_object_alloc(mrb, mrb_class_ptr(self), pp, &mrb_pipe_type));
}


static mrb_value f_pipe_implements(mrb_state *mrb, mrb_value self)
{
    int interface;

    mrb_get_args(mrb, "i", &interface);

    return mrb_bool_value(pipe_implements(*(int *)DATA_PTR(self), interface));
}


struct constant { const char *name; int value; };


#define arrlen(arr) (sizeof(arr) / sizeof(arr[0]))
#define forall(name, arr) for (struct { int i; __typeof__(arr[0]) v; } name = { 0, arr[0] }; name.i < arrlen(arr); name.v = arr[++name.i])
#define c(cnst) ((struct constant){ .name = #cnst, .value = cnst })


static mrb_value f_method_missing(mrb_state *mrb, mrb_value self)
{
    mrb_sym method;
    int value;

    int parsed = mrb_get_args(mrb, "n|i", &method, &value);

    size_t mn_len;
    const char *method_name_raw = mrb_sym2name_len(mrb, method, &mn_len);

    char method_name[mn_len + 1];
    memcpy(method_name, method_name_raw, mn_len); method_name[mn_len] = 0;

    if ((method_name[mn_len - 1] == '=') && (parsed != 2))
    {
        mrb_raisef(mrb, E_ARGUMENT_ERROR, "no value given");
        return mrb_nil_value();
    }
    else if ((method_name[mn_len - 1] != '=') && (parsed != 1))
    {
        mrb_raisef(mrb, E_NOMETHOD_ERROR, "undefined method '%S'", mrb_sym2str(mrb, method));
        return mrb_nil_value();
    }

    if (parsed == 2)
        method_name[mn_len - 1] = 0; // =

    for (int i = 0; method_name[i]; i++)
        method_name[i] = toupper(method_name[i]);

    forall(cnst, ((struct constant[]){ c(F_PRESSURE), c(F_READABLE), c(F_WRITABLE), c(F_FLUSH),
                                       c(F_DEVICE_ID), c(F_DNS_RESOLVED_IP), c(F_DNS_NSIP), c(F_DNS_DEFAULT_NSIP), c(F_DEST_MAC), c(F_SRC_MAC), c(F_MY_MAC), c(F_ETH_PACKET_TYPE),
                                       c(F_POSITION), c(F_FILESIZE), c(F_DEST_IP), c(F_SRC_IP), c(F_MY_IP), c(F_IP_TTL), c(F_IP_PROTO_TYPE), c(F_IPC_SIGNAL),
                                       c(F_INODE), c(F_MODE), c(F_NLINK), c(F_UID), c(F_GID), c(F_BLOCK_SIZE), c(F_BLOCK_COUNT), c(F_ATIME), c(F_MTIME), c(F_CTIME),
                                       c(F_CONNECTION_STATUS), c(F_ECHO), c(F_DEST_PORT), c(F_SRC_PORT), c(F_MY_PORT) }))
    {
        if (!strcmp(cnst.v.name + 2, method_name))
        {
            if (parsed == 1)
                return mrb_fixnum_value(pipe_get_flag(*(int *)DATA_PTR(self), cnst.v.value));
            else
                return mrb_fixnum_value(pipe_set_flag(*(int *)DATA_PTR(self), cnst.v.value, value));
        }
    }

    mrb_raisef(mrb, E_NOMETHOD_ERROR, "undefined method '%S'", mrb_sym2str(mrb, method));
    return mrb_nil_value();
}


void mrb_pipe_gem_init(mrb_state *mrb)
{
    struct RClass *pc = mrb_define_class(mrb, "Pipe", mrb->object_class);
    MRB_SET_INSTANCE_TT(pc, MRB_TT_DATA);

    mrb_define_class_method(mrb, pc, "create", f_create_pipe, ARGS_REQ(1) | ARGS_OPT(1));
    mrb_define_class_method(mrb, pc, "from_fd", f_from_fd, ARGS_REQ(1));

    mrb_define_method(mrb, pc, "destroy", f_destroy_pipe, ARGS_OPT(1));
    mrb_define_method(mrb, pc, "duplicate", f_duplicate_pipe, ARGS_OPT(1));
    mrb_define_method(mrb, pc, "send", f_stream_send, ARGS_REQ(1) | ARGS_OPT(1));
    mrb_define_method(mrb, pc, "receive", f_stream_receive, ARGS_OPT(2));
    mrb_define_method(mrb, pc, "implements", f_pipe_implements, ARGS_REQ(1));
    mrb_define_method(mrb, pc, "implements?", f_pipe_implements, ARGS_REQ(1));

    mrb_define_method(mrb, pc, "method_missing", f_method_missing, ARGS_OPT(1));


    forall(cnst, ((struct constant[]){ c(O_RDONLY), c(O_WRONLY), c(O_RDWR), c(O_CREAT), c(O_JUST_STAT), c(O_TRUNC), c(O_APPEND), c(O_NOFOLLOW) }))
        mrb_define_const(mrb, pc, cnst.v.name, mrb_fixnum_value(cnst.v.value));

    forall(cnst, ((struct constant[]){ c(O_TRANS_NC), c(O_NONBLOCK), c(O_BLOCKING) }))
        mrb_define_const(mrb, pc, cnst.v.name, mrb_fixnum_value(cnst.v.value));

    forall(cnst, ((struct constant[]){ c(O_NONE), c(F_MOUNT_FILE) }))
        mrb_define_const(mrb, pc, cnst.v.name, mrb_fixnum_value(cnst.v.value));

    forall(cnst, ((struct constant[]){ c(I_DEVICE_CONTACT), c(I_DNS), c(I_ETHERNET), c(I_FILE), c(I_FS), c(I_IP), c(I_SIGNAL), c(I_STATABLE), c(I_TCP), c(I_TTY), c(I_UDP) }))
        mrb_define_const(mrb, pc, cnst.v.name, mrb_fixnum_value(cnst.v.value));

    forall(cnst, ((struct constant[]){ c(F_PRESSURE), c(F_READABLE), c(F_WRITABLE), c(F_FLUSH),
                                       c(F_DEVICE_ID), c(F_DNS_RESOLVED_IP), c(F_DNS_NSIP), c(F_DNS_DEFAULT_NSIP), c(F_DEST_MAC), c(F_SRC_MAC), c(F_MY_MAC), c(F_ETH_PACKET_TYPE),
                                       c(F_POSITION), c(F_FILESIZE), c(F_DEST_IP), c(F_SRC_IP), c(F_MY_IP), c(F_IP_TTL), c(F_IP_PROTO_TYPE), c(F_IPC_SIGNAL),
                                       c(F_INODE), c(F_MODE), c(F_NLINK), c(F_UID), c(F_GID), c(F_BLOCK_SIZE), c(F_BLOCK_COUNT), c(F_ATIME), c(F_MTIME), c(F_CTIME),
                                       c(F_CONNECTION_STATUS), c(F_ECHO), c(F_DEST_PORT), c(F_SRC_PORT), c(F_MY_PORT) }))
    {
        mrb_define_const(mrb, pc, cnst.v.name, mrb_fixnum_value(cnst.v.value));
    }

    forall(cnst, ((struct constant[]){ c(IPC_SIGNAL) }))
        mrb_define_const(mrb, pc, cnst.v.name, mrb_fixnum_value(cnst.v.value));
}


void mrb_pipe_gem_final(mrb_state *mrb)
{
}
