function love.conf(t)
  t.console = true
  t.window.name = 'love.test'
  t.window.width = 360
  t.window.height = 240
  t.window.resizable = true
  t.renderers = {"opengl"}
  t.modules.audio = true
  t.modules.data = true
  t.modules.event = true
  t.modules.filesystem = true
  t.modules.font = true
  t.modules.graphics = true
  t.modules.image = true
  t.modules.math = true
  t.modules.objects = true
  t.modules.physics = true
  t.modules.sound = true
  t.modules.system = true
  t.modules.thread = true
  t.modules.timer = true
  t.modules.video = true
  t.modules.window = true
end
