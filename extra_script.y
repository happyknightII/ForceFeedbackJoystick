Import("env")
env.AddPostAction(
    "buildprog",
    lambda *args, **kwargs: env.Execute("pio run --target compiledb")
)
