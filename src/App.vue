<template>
  <div id="app" style="height: 100%;width: 100%;">
    <bousaiApp />
  </div>
</template>

<script>
import bousaiApp from '@/components/bousaiApp.vue'

export default {
  name: 'App',
  components: {
    bousaiApp
  }
}
</script>

<style>
html, body{
  height: 100%;
  overflow: hidden;
  padding: 0;
  margin: 0;
}
</style>