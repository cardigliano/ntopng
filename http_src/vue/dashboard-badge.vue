<!--
  (C) 2013-22 - ntop.org
-->

<template>
<div class="d-flex align-items-center justify-content-between">
    <div>
        <a :href="link_url">
            <h4 class="fw-normal text-white">{{ counter }}</h4>
            <p class="subtitle text-white text-sm text mb-0 h5">{{ name }}</p>
        </a>
    </div>
    <div class="flex-shrink-0 ms-3">
        <i class="text-white" :class="icon"></i>
    </div>
</div>
</template>

<script setup>
import { ref, onMounted, onBeforeMount, watch } from "vue";
import { ntopng_custom_events, ntopng_events_manager } from "../services/context/ntopng_globals_services";
import formatterUtils from "../utilities/formatter-utils";
import NtopUtils from "../utilities/ntop-utils";

const _i18n = (t) => i18n(t);

const component_id = ref('empty_component');

const counter = ref('')
const name = ref('')
const icon = ref('')
const link_url = ref('#')

const props = defineProps({
    id: String,          /* Component ID */
    i18n_title: String,  /* Title (i18n) */
    ifid: Number,        /* Interface ID */
    epoch_begin: Number, /* Time interval begin */
    epoch_end: Number,   /* Time interval end */
    max_width: Number,   /* Component Width (4, 8, 12) */
    max_height: Number,  /* Component Hehght (4, 8, 12)*/
    params: Object,      /* Component-specific parameters from the JSON template definition */
    get_component_data: Function /* Callback to request data (REST) */
});

/* Watch - detect changes on epoch_begin / epoch_end and refresh the component */
watch(() => [props.epoch_begin, props.epoch_end], (cur_value, old_value) => {
  refresh_component();
}, { flush: 'pre'});

onBeforeMount(() => {
  init();
});

onMounted(() => {
});

function init() {
  if (props.params.i18n_name) {
    name.value = _i18n(props.params.i18n_name);
  }

  if (props.params.icon) {
    icon.value = props.params.icon + ' fa-2xl';
  }

  refresh_component();
}

async function refresh_component() {
  /* Refresh component */

  if (props.params.url) {

    const url_params = {
      ifid: props.ifid,
      epoch_begin: props.epoch_begin,
      epoch_end: props.epoch_end,
      ...props.params.url_params
    }
    const query_params = ntopng_url_manager.obj_to_url_params(url_params);

    // let data = await ntopng_utility.http_request(`${http_prefix}${props.params.url}?${query_params}`);
    let data = await props.get_component_data(`${http_prefix}${props.params.url}`, query_params);

    /* TODO handle dot-separated path for non-flat json */
    let counter_value = data[props.params.counter_path];

    let counter_formatter = data[props.params.counter_formatter];
    if (!counter_formatter) {
      counter_formatter = "number";
    }

    let formatCounter = formatterUtils.getFormatter(counter_formatter);
    counter.value = formatCounter(counter_value)

    if (props.params.link) {
      const link_url_params = {
        ifid: props.ifid,
        epoch_begin: props.epoch_begin,
        epoch_end: props.epoch_end,
        ...props.params.link.url_params
      }

      const link_query_params = ntopng_url_manager.obj_to_url_params(link_url_params);
      link_url.value = `${http_prefix}${props.params.link.url}?${link_query_params}`;
    }
  }
}
</script>
