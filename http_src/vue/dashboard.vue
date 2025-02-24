<!-- (C) 2023 - ntop.org -->
<template>
<div class='row'>
  <!-- <Dropdown v-for="(t, t_index) in top_table_array" -->
  <!--           :ref="el => { top_table_dropdown_array[t_index] = el }"> -->
  <!--   <template v-slot:title> -->
  <!--     <Spinner :show="t.show_spinner" size="1rem" class="me-1"></Spinner> -->
  <!--     <a class="ntopng-truncate" :title="t.title">{{ t.label }}</a> -->
  <!--   </template> -->
  <!--   <template v-slot:menu> -->
  <!--     <a v-for="opt in t.options" style="cursor:pointer; display: block;" -->
  <!--        @click="add_top_table_filter(opt, $event)" class="ntopng-truncate tag-filter " -->
  <!--        :title="opt.value">{{ opt.label }}</a> -->
  <!--   </template>     -->
  <!-- </Dropdown> -->
  
  <DateTimeRangePicker v-if="enable_date_time_range_picker"
                       class="dontprint"
                       :disabled_date_picker="disable_date_time_picker"
                       id="dashboard-date-time-picker"
                       :round_time="true"
                       min_time_interval_id="min"
                       @epoch_change="set_components_epoch_interval">
    <template v-slot:begin>
      <div class="me-2">
        <SelectSearch v-model:selected_option="selected_report_template" :options="reports_templates"
                      @select_option="select_report_template">
        </SelectSearch>
      </div>
    </template>
    <template v-slot:extra_buttons>
      <button class="btn btn-link btn-sm"
              type="button"
              @click="show_store_report_modal" :title="_i18n('dashboard.store')">
        <i class="fa-solid fa-floppy-disk"></i>
      </button>
      <button class="btn btn-link btn-sm"
              type="button"
              @click="show_open_report_modal" :title="_i18n('dashboard.open')">
        <i class="fa-solid fa-folder-open"></i>
      </button>
      <button class="btn btn-link btn-sm"
              type="button"
              @click="download_report" :title="_i18n('download')">
        <i class="fa-solid fa-file-arrow-down"></i>
      </button>
      <button class="btn btn-link btn-sm"
              type="button"
              @click="show_upload_report_modal" :title="_i18n('upload')">
        <i class="fa-solid fa-file-arrow-up"></i>
      </button>
      <button class="btn btn-link btn-sm"
              type="button"
              @click="print_report" :title="_i18n('dashboard.print')">
        <i class="fas fa-print"></i>
      </button>
    </template>
  </DateTimeRangePicker>
  
  <div v-if="enable_report_title" class="mt-3" style="margin-bottom:-0.5rem;"><h3 style="text-align:center;">Report: {{selected_report_template.value}}</h3></div>
  <div ref="report_box" class="row" :key="components">
    <template v-for="c in components">
      <Box style="min-width:20rem;"
           :color="c.color"
           :width="c.width" 
           :height="c.height">
        <template v-slot:box_title>
          <div v-if="c.i18n_name" class="dashboard-component-title">
            <h4>
              {{ _i18n(c.i18n_name) }}
              <span style="color: gray">
                {{ c.time_offset ? _i18n('dashboard.time_ago.' + c.time_offset) : '' }}
              </span>
            </h4>
          </div>
        </template>
        <template v-slot:box_content>
          <component :is="components_dict[c.component]"
                     :id="c.component_id"
                     :style="component_custom_style(c)"
                     :epoch_begin="c.epoch_begin"
                     :epoch_end="c.epoch_end"
                     :i18n_title="c.i18n_name"
                     :ifid="c.ifid ? c.ifid : context.ifid"
                     :max_width="c.width"
                     :max_height="c.height"
                     :params="c.params"
                     :get_component_data="get_component_data_func(c)"
                     :csrf="context.csrf">
          </component>
        </template>
        <template v-slot:box_footer>
          <span v-if="c.component != 'empty' && c.i18n_name && !disable_date" style="color: lightgray;font-size:12px;">
            {{ component_interval(c) }}
          </span>
        </template>
      </Box>
    </template>
  </div>
</div> <!-- div row -->
<ModalSave ref="modal_store_report"
           :get_suggested_file_name="get_suggested_report_name"
           :store_file="store_report"
           :csrf="context.csrf"
           :title="_i18n('dashboard.store')">
</ModalSave>
<ModalOpen ref="modal_open_report"
           :list_files="list_reports"
           :open_file="open_report"
           :delete_file="delete_report"
           :csrf="context.csrf"
           :title="_i18n('dashboard.open')"
           :file_title="_i18n('report.report_name')">
</ModalOpen>
<ModalUpload ref="modal_upload_report"
             :upload_file="upload_report"
             :title="_i18n('upload')"
             :file_title="_i18n('report.file')">
</ModalUpload>
</template>

<script setup>
import { ref, onMounted, onBeforeMount, computed, nextTick } from "vue";
import { ntopng_status_manager, ntopng_custom_events, ntopng_url_manager, ntopng_utility, ntopng_events_manager, ntopng_sync } from "../services/context/ntopng_globals_services";

import { default as DateTimeRangePicker } from "./date-time-range-picker.vue";

import { default as ModalSave } from "./modal-file-save.vue";
import { default as ModalOpen } from "./modal-file-open.vue";
import { default as ModalUpload } from "./modal-file-upload.vue";

import { default as Box } from "./dashboard-box.vue";

import { default as EmptyComponent } from "./dashboard-empty.vue";
import { default as TableComponent } from "./dashboard-table.vue";
import { default as BadgeComponent } from "./dashboard-badge.vue";
import { default as PieComponent } from "./dashboard-pie.vue";
import { default as TimeseriesComponent } from "./dashboard-timeseries.vue";
import { default as SankeyComponent } from "./dashboard-sankey.vue";
import { default as SelectSearch } from "./select-search.vue";
import { default as Dropdown } from "./dropdown.vue"

const _i18n = (t) => i18n(t);
const timeframes_dict = ntopng_utility.get_timeframes_dict();

const props = defineProps({
    context: Object,
});

const components_dict = {
    "badge": BadgeComponent,
    "empty": EmptyComponent,
    "pie": PieComponent,
    "timeseries": TimeseriesComponent,
    "table": TableComponent,
    "sankey": SankeyComponent,
}

const page_id = "page-dashboard";
const default_ifid = props.context.ifid;
const report_box = ref(null);

const modal_store_report = ref(null);
const modal_open_report = ref(null);
const modal_upload_report = ref(null);

const main_epoch_interval = ref(null);

const components = ref([]);

const reports_templates = ref([]);
const selected_report_template = ref({});

let components_info = {};
let data_from_backup = false;
let printable = false;

const enable_date_time_range_picker = computed(() => {
    return props.context.page == "report";
});

const disable_date = computed(() => {
    return selected_report_template.value?.toolbox?.time?.hide == true;
});

const disable_date_time_picker = computed(() => {
    const disabled = selected_report_template.value.is_open_report == true
          || disable_date.value;
    return disabled;
});


const enable_report_title = computed(() => {
    const enable = selected_report_template.value.is_open_report == true;
    return enable;
});

const component_custom_style = computed(() => {
    return (c) => {
        if (c.params.custom_style != null && (!printable || c.params.custom_print_style)) {
            return c.params.custom_style;
        } else if (c.params.custom_print_style && printable == true) {
            return c.params.custom_print_style;
        }
        return "";
    };
});

const component_interval = computed(() => {
    return (c) => {
        const time_interval_string = get_time_interval_string(c.epoch_begin, c.epoch_end);
        return time_interval_string;
    };
});

onBeforeMount(async () => {
    let epoch_interval = null;
    printable = ntopng_url_manager.get_url_entry("printable") == "true";
    if (props.context.page == "report") {
        epoch_interval = ntopng_utility.check_and_set_default_time_interval(undefined, undefined, true, "min");
        main_epoch_interval.value = epoch_interval;
    }
    await set_templates_list();
    let report_name = ntopng_url_manager.get_url_entry("report_name");
    if (report_name != null && report_name != "") {
        /* Report name provided - open a report backup */
        await open_report(report_name);
    } else {
        /* Load a template and build a new report */
        await load_components(epoch_interval, selected_report_template.value.value);
    }
    if (printable == true) {
        set_report_title();
        // await nextTick();
        // ntopng_sync.ready("print_report");
    }
});

onMounted(async () => {
    if (props.context.page == "dashboard") {
        start_dashboard_refresh_loop();
    }
    // if (printable == true) {
    //     await ntopng_sync.on_ready("print_report");
    // }
});

async function set_templates_list() {
    const url_request = props.context.template_list_endpoint;
    let res = await ntopng_utility.http_request(url_request);
    if (res?.list == null) { return; }
    
    reports_templates.value = res.list.map((t) => {
        return {
            value: t.name,
            label: t.label,
            disabled: false,
            toolbox: t.toolbox,
            is_open_report: false,
        };
    });
    const report_template_value = ntopng_url_manager.get_url_entry("report_template") || props.context.template;
    selected_report_template.value = reports_templates.value.find((t) => t.value == report_template_value);
    if (selected_report_template.value == null) {
        selected_report_template.value = reports_templates.value[0];
    }
}

let dasboard_loop_interval;

/* Dashboard update interval/frequency */
const loop_interval = 10 * 1000;

function start_dashboard_refresh_loop() {
    dasboard_loop_interval = setInterval(() => {
        set_components_epoch_interval();
    }, loop_interval);
}

function set_components_epoch_interval(epoch_interval) {
    if (epoch_interval) {
        main_epoch_interval.value = epoch_interval;
    }
    
    components.value.forEach((c, i) => {
        update_component_epoch_interval(c, epoch_interval);
    });
}

async function load_components(epoch_interval, template_name) {
    /* Enable REST calls */    
    data_from_backup = false;
    
    let url_request = `${props.context.template_endpoint}?template=${template_name}`;
    let res = await ntopng_utility.http_request(url_request);
    components.value = res.list.filter((c) => components_dict[c.component] != null)
        .map((c, index) => {
            let c_ext = {
                component_id: get_component_id(c.id, index),
                ...c
            };
            update_component_epoch_interval(c_ext, epoch_interval);
            return c_ext;
        });
    await nextTick();
}

function update_component_epoch_interval(c, epoch_interval) {
    const interval_seconds = timeframes_dict[c.time_window || "5_min"];
    if (epoch_interval == null) {
        const epoch_end = ntopng_utility.get_utc_seconds();
        epoch_interval = { epoch_begin: epoch_end - interval_seconds, epoch_end: epoch_end };
    }
    const utc_offset = timeframes_dict[c.time_offset] || 0;
    c.epoch_begin = epoch_interval.epoch_begin - utc_offset;
    c.epoch_end = epoch_interval.epoch_end - utc_offset;
}

function select_report_template() {
    if (printable == true) {
        set_report_title();
    }
    if (selected_report_template.value.is_open_report == true) {
        return;
    }
    components_info = {};
    update_templates_list();
    const global_status = ntopng_status_manager.get_status(true);
    let epoch_interval = { epoch_begin: global_status.epoch_begin, epoch_end: global_status.epoch_end };
    if (data_from_backup == true) { // last report selected it was a saved report and then we must to restore default timestamp
        epoch_interval = ntopng_utility.set_default_time_interval(undefined, "min");
        ntopng_events_manager.emit_event(ntopng_events.EPOCH_CHANGE, epoch_interval, props.context.page);
    }
    load_components(epoch_interval, selected_report_template.value.value);
}

function get_component_id(id, index) {
    return `${page_id}_${id}_${index}`;
}

function show_store_report_modal() {
    modal_store_report.value.show();
}

function show_open_report_modal() {
    modal_open_report.value.show();
}

function show_upload_report_modal() {
    modal_upload_report.value.show();
}

function get_suggested_report_name() {
    return "report-" + ntopng_utility.from_utc_to_server_date_format(main_epoch_interval.value.epoch_end * 1000, 'DD-MM-YYYY');
}

const upload_report = async (content_string) => {
    let content = JSON.parse(content_string);
    set_report(content, content.name);
    ntopng_url_manager.delete_key_from_url("report_name");
}

function set_report(content, name) {
    update_templates_list(name);
    const epoch_status = { epoch_begin: content.epoch_begin, epoch_end: content.epoch_end };
    ntopng_events_manager.emit_event(ntopng_events.EPOCH_CHANGE, epoch_status, props.context.page);
    load_report(content);
}

const list_reports = async () => {
    let url = `${http_prefix}/lua/pro/rest/v2/get/report/backup/list.lua?ifid=${props.context.ifid}`;
    let files_obj = await ntopng_utility.http_request(url);
    let files = ntopng_utility.object_to_array(files_obj);
    
    /* Return array of [{ name: String, epoch: Number }, ...] */
    
    return files;
}

const load_report = async (content) => {
    let tmp_name = content.name;
    let tmp_epoch_interval = {
        epoch_begin: content.epoch_begin,
        epoch_end:   content.epoch_end
    };
    let tmp_template = content.template;
    let tmp_components_data = content.data;
    
    let tmp_components_info = {};
    for (let key in tmp_components_data) {
        let info = {
            data: tmp_components_data[key],
        };
        tmp_components_info[key] = info;
    }
    
    /* Disable REST calls */    
    data_from_backup = true;
    
    /* Set the cached data from the backup */
    components_info = tmp_components_info;
    
    /* Change the components (template) from the backup */
    components.value = tmp_template;
    
    /* Change the time interval on components */
    set_components_epoch_interval(tmp_epoch_interval);
}

const open_report = async (file_name) => {
    let url = `${http_prefix}/lua/pro/rest/v2/get/report/backup/file.lua?ifid=${props.context.ifid}&report_name=${file_name}`;
    let content = await ntopng_utility.http_request(url);
    set_report(content, file_name);
}

function update_templates_list(report_name_to_open) {
    reports_templates.value = reports_templates.value.filter((t) => t.is_open_report == false);
    if (report_name_to_open == null) { // in this case is selected a report_template
        ntopng_url_manager.set_key_to_url("report_template", selected_report_template.value.value);
        ntopng_url_manager.delete_key_from_url("report_name");
        return;
    }
    
    let t_entry = {
        value: report_name_to_open,
        label: _i18n("dashboard.custom"),
        disabled: false,
        toolbox: null,
        is_open_report: true,
    };
    reports_templates.value.push(t_entry);
    selected_report_template.value = t_entry;
    ntopng_url_manager.set_key_to_url("report_name", selected_report_template.value.value);
    ntopng_url_manager.delete_key_from_url("report_template");
}

const delete_report = async (file_name) => {
    let success = false;
    
    let params = {
        csrf: props.context.csrf,
        ifid: props.context.ifid,
    	report_name: file_name
    };
    
    let url = `${http_prefix}/lua/pro/rest/v2/delete/report/backup/file.lua`;
    try {
    	let headers = {
    	    'Content-Type': 'application/json'
    	};
    	await ntopng_utility.http_request(url, { method: 'post', headers, body: JSON.stringify(params) });
        success = true;
    } catch(err) {
    	console.error(err);
    }
    
    return success;
}

/* Dump report content - keep in sync with dashboard_utils.build_report (lua) */
const serialize_report = async (name) => {
    
    let components_data = {};
    for (var key in components_info) {
        components_data[key] = await components_info[key].data;
    }
    
    let content = {
        version: "1.0", // Report dump version
        name: name,
        epoch_begin: main_epoch_interval.value.epoch_begin,
        epoch_end: main_epoch_interval.value.epoch_end,
        template: components.value,
        data: components_data
    };
    
    return JSON.stringify(content);
}

const store_report = async (file_name) => {
    let success = false;
    
    let data = {
        csrf: props.context.csrf,
        ifid: props.context.ifid,
	report_name: file_name,
        content: await serialize_report(file_name)
    };
    
    let url = `${http_prefix}/lua/pro/rest/v2/add/report/backup/file.lua`;
    try {
	let headers = {
	    'Content-Type': 'application/json'
	};
	await ntopng_utility.http_request(url, { method: 'post', headers, body: JSON.stringify(data) });
        success = true;
    } catch(err) {
	console.error(err);
    }
    
    return success;
}

async function download_report() {
    var name = get_suggested_report_name();
    var filename = name + '.json';
    var content = await serialize_report(name);
    var element = document.createElement('a');
    element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(content));
    element.setAttribute('download', filename);
    element.style.display = 'none';
    document.body.appendChild(element);
    element.click();
    document.body.removeChild(element);
}

function print_report() {
    if (printable == true) {
        window.print();
        return false;
    }

    let url_params_obj = ntopng_url_manager.get_url_object();
    url_params_obj.printable = true;
    const params = ntopng_url_manager.obj_to_url_params(url_params_obj);
    let url = `${http_prefix}/lua/pro/reportng.lua?${params}`;
    //const print_key = "printable";
    //ntopng_url_manager.set_key_to_url(print_key, true);
    ntopng_url_manager.open_new_window(url);
    //ntopng_url_manager.delete_key_from_url(print_key);
    // $(report_box.value).print({mediaPrint: true, timeout: 1000}); 
    // $(report_box.value).print();
}

function get_time_interval_string(epoch_begin, epoch_end) {
    if (disable_date.value == true) { return ""; }
    
    const epoch_begin_msec = epoch_begin * 1000;    
    const epoch_end_msec = epoch_end * 1000;
    
    const begin_date = ntopng_utility.from_utc_to_server_date_format(epoch_begin_msec, 'DD/MM/YYYY');
    const begin_time = ntopng_utility.from_utc_to_server_date_format(epoch_begin_msec, 'HH:mm:ss');
    
    const end_date = ntopng_utility.from_utc_to_server_date_format(epoch_end_msec, 'DD/MM/YYYY');
    const end_time = ntopng_utility.from_utc_to_server_date_format(epoch_end_msec, 'HH:mm:ss');
    
    const begin = `${begin_date} ${begin_time}`;
    const end = (begin_date == end_date) ? `${end_time}` : `${end_date} ${end_time}`;
    return `${begin} - ${end}`;
}

function set_report_title() {
    const epoch_interval = ntopng_status_manager.get_status(true);
    const time_interval_string = get_time_interval_string(epoch_interval.epoch_begin, epoch_interval.epoch_end);
    document.title = `ntopng - Report ${selected_report_template.value.label} ${time_interval_string}`;
}

/* Callback to request REST data from components */
function get_component_data_func(component) {
    const get_component_data = async (url, url_params, post_params) => {
        
        let info = {};
        if (data_from_backup) {
            if (!components_info[component.component_id]) { /* Safety check */
                console.error("No data for " + component.component_id);
                info.data = {};
            } else {
                info = components_info[component.component_id];
            }
        } else {

            /* Check if there is already a promise for the same request */
            if (components_info[component.component_id]) {
                info = components_info[component.component_id];
                if (info.data) {
                    await info.data; /* wait in case of previous pending requests */
                }
            }

            const data_url = `${url}?${url_params}`;

            if (post_params) {
                info.data = ntopng_utility.http_post_request(data_url, post_params);
            } else {
                info.data = ntopng_utility.http_request(data_url);
            }
        
            components_info[component.component_id] = info;
        }
        return info.data;
    };
    return get_component_data;
}

</script>

<style scoped>
@media print {
.dontprint { display: none; }
.pagebreak-begin { page-break-before: always; }
.pagebreak-end { page-break-after: always !important; }
.print-element-class { page-break-inside:avoid !important; page-break-after:auto }
}

/* @media print and (orientation: portrait) and (max-width: 297mm){ */
/*     .col-4 { */
/*         width: 50% !important; */
/*         flex: 0 0 auto; */
/*     } */
/* } */
@page {
    /* size: A3 landscape; */
    /* position:absolute; width:100%; top:0;left:0;right:0;bottom:0; padding:0; margin:-1px; */
}

/* Print on A4 */
@media print and (max-width: 297mm) and (min-width: 210mm) {
    /* .row { */
    /*         padding-left: 0; */
    /*         padding-right: 0; */
    /*         margin-left: -10rem; */
    /*         margin-right: 0; */
    /* } */
   .col-4 {
        width: 50% !important;
        flex: 0 0 auto;
    }
}

/* Print on A5 (commented out as this is not working on Chrome/Safari) */

/* @media print and (max-width: 148mm){ */
/*     .col-4 { */
/*         width: 100% !important; */
/*         flex: 0 0 auto; */
/*     } */
/*     .col-6 { */
/*         width: 100% !important; */
/*         flex: 0 0 auto; */
/*     } */
/* } */

.align-center {
}
</style>
